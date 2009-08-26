#include "MSNManager.h"

#include <libim/Protocol.h>
#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <UTF8.h>
#include <algorithm>

#include "MSNConnection.h"
#include "MSNSBConnection.h"
#include "MSNHandler.h"

void PrintHex(const unsigned char* buf, size_t size) {
//	if ( g_verbosity_level != liDebug ) {
//		// only print this stuff in debug mode
//		return;
//	}
	
	int i = 0;
	int j = 0;
	int breakpoint = 0;

	for(;i < (int)size; i++) {
		fprintf(stdout, "%02x ", (unsigned char)buf[i]);
		breakpoint++;	

		if(!((i + 1)%16) && i) {
			fprintf(stdout, "\t\t");
			for(j = ((i+1) - 16); j < ((i+1)/16) * 16; j++)	{
				if(buf[j] < 30) {
					fprintf(stdout, ".");
				} else {
					fprintf(stdout, "%c", (unsigned char)buf[j]);
				};
			}
			fprintf(stdout, "\n");
			breakpoint = 0;
		}
	}
	
	if(breakpoint == 16) {
		fprintf(stdout, "\n");
		return;
	}

	for(; breakpoint < 16; breakpoint++) {
		fprintf(stdout, "   ");
	}
	
	fprintf(stdout, "\t\t");

	for(j = size - (size%16); j < (int)size; j++) {
		if(buf[j] < 30) {
			fprintf(stdout, ".");
		} else {
			fprintf(stdout, "%c", (unsigned char)buf[j]);
		};
	}
	
	fprintf(stdout, "\n");
}

void remove_html( char * msg )
{
	bool is_in_tag = false;
	int copy_pos = 0;
	
	char * copy = new char[strlen(msg)+1];
	
	for ( int i=0; msg[i]; i++ )
	{
		switch ( msg[i] )
		{
			case '<':
				is_in_tag = true;
				break;
			case '>':
				is_in_tag = false;
				break;
			default:
				if ( !is_in_tag )
				{
					copy[copy_pos++] = msg[i];
				}
		}
	}
	
	copy[copy_pos] = 0;
	
	strcpy(msg, copy);
}

void
wait_for_threads( std::list<thread_id> threads )
{
	int32 res=0;
	
	for ( std::list<thread_id>::iterator i = threads.begin(); i != threads.end(); i++ )
	{
		wait_for_thread( *i, &res );
	}
}

MSNManager::MSNManager(MSNHandler *handler)
: BLooper("MSNManager looper") {	
	fConnectionState = otOffline;
	
	fHandler = handler;
	fDisplayName = "IMKit User";
	fPassport = "";
	fPassword = "";
	fNoticeCon = NULL;
};

MSNManager::~MSNManager(void) {
	LogOff();
}

status_t MSNManager::Login(const char *server, uint16 port, const char *passport,
	const char *password, const char *displayname) {
	
	if ((passport == NULL) || (password == NULL)) {
		LOG(kProtocolName, liHigh, "MSNManager::Login: passport or password not "
			"set");
		return B_ERROR;
	}
	
	fPassport = passport;
	fPassword = password;
	fDisplayName = displayname;
	
	// set up connection pool
	while ( fConnectionPool.size() < 3 )
		fConnectionPool.push_back( new MSNConnection() );

	if (fConnectionState == otOffline) {
		BMessage msg(msnmsgProgress);
		msg.AddString("id", "MSN Login");
		msg.AddString("message", "MSN: Connecting..");
		msg.AddFloat("progress", 0.10 );
		
		BMessenger(this).SendMessage(&msg);
		
		if (fNoticeCon == NULL) {
			fNoticeCon = *fConnectionPool.begin();
			fConnectionPool.pop_front();
			fConnectionPool.push_back( new MSNConnection() );
			fNoticeCon->SetTo( server, port, this );
		};
		
		if ( !fNoticeCon || !fNoticeCon->IsConnected() )
		{
			LOG(kProtocolName, liDebug, "MSNManager::Login: Error connecting");
			
			return B_ERROR;
		}
		
		Command *command = new Command("VER");
		command->AddParam(kProtocolsVers);
		fNoticeCon->Send(command);
		
		return B_OK;
	} else {
		LOG(kProtocolName, liDebug, "MSNManager::Login: Already online");
		return B_ERROR;
	};
};

void MSNManager::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case msnMessageRecveived: {
			BString passport = msg->FindString("passport");
			BString message = msg->FindString("message");

			fHandler->MessageFromUser(passport.String(), message.String());
		} break;
		
		case msnmsgStatusChanged: {
			uint8 status = msg->FindInt8("status");
			BString passport = msg->FindString("passport");
			fHandler->StatusChanged(passport.String(), (online_types)status);
		} break;
		
		case msnmsgOurStatusChanged: {
			uint8 status = msg->FindInt8("status");
			fHandler->StatusChanged(fPassport.String(), (online_types)status);
			fConnectionState = status;
		} break;
		
		case msnmsgNewConnection: {
			int16 port = 0;
			char *host = NULL;
			const char *type = NULL;

			if (msg->FindString("host", (const char **)&host) != B_OK) {
				LOG(kProtocolName, liLow, "Got a malformed new connection message"
					" (Host)");
				return;
			};
			if (msg->FindInt16("port", &port) != B_OK) {
				LOG(kProtocolName, liLow, "Got a malformed new connection message"
					" (Port)");
				return;
			};
			
			msg->FindString("type", &type);
			
			LOG(kProtocolName, liDebug, "Got a new connection to \"%s\":%i of type \"%s\"", host,
				port, type);
			
			if (strcmp(type, "NS") == 0) {
				//MSNConnection *con = new MSNConnection(host, port, this);
				MSNConnection *con = *fConnectionPool.begin();
				fConnectionPool.pop_front();
				fConnectionPool.push_back( new MSNConnection() );
				con->SetTo( host, port, this );
				//con->Lock();
				
				Command *command = new Command("VER");
				command->AddParam(kProtocolsVers);
					
				con->Send(command);
				
				fNoticeCon = con;
					
				LOG(kProtocolName, liDebug, "  Set fNoticeCon");
				
				return;
			}
			
			if (strcmp(type, "RNG") == 0) {
				MSNSBConnection *con = new MSNSBConnection(host, port, this);
				//con->Run();
				
				const char *auth = msg->FindString("authString");
				const char *sessionID = msg->FindString("sessionID");
				//const char *inviter = msg->FindString("inviterPassport");
				
				Command *command = new Command("ANS");
				command->AddParam(fPassport.String());
				command->AddParam(auth);
				command->AddParam(sessionID);
				
				con->Send(command);
				
				fConnections.push_back( con );
				
				return;
			};
			
			if (strcmp(type, "SB") == 0) {
				MSNSBConnection *con = new MSNSBConnection(host, port, this);
				//con->Run();
				
				const char *authString = msg->FindString("authString");
				
				Command *command = new Command("USR");
				command->AddParam(Passport());
				command->AddParam(authString);
				
				con->Send(command);
				
				waitingmsgmap::iterator it = fWaitingSBs.begin();
				
				if (it != fWaitingSBs.end()) {
					BString passport = (*it).second.first;
					Command *message = (*it).second.second;
					
					Command *cal = new Command("CAL");
					cal->AddParam(passport.String());
					
					con->Send(cal, qsOnline);
					con->SendMessage(message);
					
					// assume it's a MSG here..
					fConnections.push_back( con );
					
					fWaitingSBs.erase( (*it).first );
				};
			};
		} break;

		case msnmsgCloseConnection: {
			MSNConnection *con = NULL;
			msg->FindPointer("connection", (void **)&con);
			
			if (con != NULL) {
				LOG(kProtocolName, liLow, "Connection (%lX) closed", con);
				
				std::list<thread_id> threads;
				
				if (con == fNoticeCon) {
					LOG(kProtocolName, liLow, "  Notice connection closed, go offline");
					connectionlist::iterator i;
					for (i = fConnections.begin(); i != fConnections.end(); i++) {
						threads.push_back( (*i)->Thread() );
						Command * bye = new Command("OUT");
						bye->UseTrID(false);
						(*i)->Send(bye, qsImmediate);
						BMessenger(*i).SendMessage(B_QUIT_REQUESTED);
					};
					
					fHandler->StatusChanged(Passport(), otOffline);
					
					fNoticeCon = NULL;
					fConnectionState = otOffline;
					LOG(kProtocolName, liDebug, "  Unset fNoticeCon");
				};
				
				threads.push_back( con->Thread() );
				BMessenger(con).SendMessage(B_QUIT_REQUESTED);
				
				connectionlist::iterator i = find(fConnections.begin(), fConnections.end(), con);
				
				if ( i != fConnections.end() )
					fConnections.erase( i );
				
				wait_for_threads( threads );
			};
		} break;
		
		case msnmsgRemoveConnection: {
			MSNConnection *con = NULL;
			msg->FindPointer("connection", (void **)&con);
			
			if (con != NULL) {
				connectionlist::iterator i = find(fConnections.begin(), fConnections.end(), con);
				
				LOG(kProtocolName, liLow, "Connection (%lX) removed", con);
				
				if ( i != fConnections.end() )
				{ // don't call anything in con, it's already deleted.
					fConnections.erase( i );
				}
				
				if ( con == fNoticeCon ) {
					std::list<thread_id> threads;
					
					LOG(kProtocolName, liLow, "  Notice connection closed, going offline");
					connectionlist::iterator i;
					for (i = fConnections.begin(); i != fConnections.end(); i++) {
						threads.push_back( (*i)->Thread() );
						Command * bye = new Command("OUT");
						bye->UseTrID(false);
						(*i)->Send(bye, qsImmediate);
						BMessenger(*i).SendMessage(B_QUIT_REQUESTED);
					}
					
					wait_for_threads( threads );
					
					fHandler->StatusChanged(Passport(), otOffline);
					
					fNoticeCon = NULL;
					fConnectionState = otOffline;
					
					LOG(kProtocolName, liDebug, "  Unset fNoticeCon");
				}
			};
		} break;
		
		case msnAuthRequest: {
			// check if that person is already in our AL before asking.
			BString display = msg->FindString("displayname");
			BString passport = msg->FindString("passport");
			list_types listType = (list_types)msg->FindInt8("list");
			
			Buddy *bud = new Buddy(passport.String());
			bud->Lists(listType);
			fWaitingAuth[passport] = bud;
			
			fHandler->AuthRequest(listType, passport.String(), display.String());
		} break;
		
		case msnContactInfo: {
			UpdateContactInfo( msg );
		} break;
		
		case msnmsgError: {
			fHandler->Error( msg->FindString("error") );
		} break;
		
		case msnmsgProgress: {
			float progress=0.0;
			if ( msg->FindFloat("progress", &progress) != B_OK )
			{
				LOG(kProtocolName, liHigh, "Malformed msnmsgProgress message received by MSNManager");
				break;
			}
			
			fHandler->Progress( msg->FindString("id"), msg->FindString("message"), progress );
		} break;
		
		default: {
			BLooper::MessageReceived(msg);
		};
	};
}

// -- Interface

status_t MSNManager::MessageUser(const char *passport, const char *message) {
	if ((fConnectionState != otOffline) && (fConnectionState != otConnecting)) {
		if (fNoticeCon == NULL) {
			LOG(kProtocolName, liDebug, "Could not message %s, fNoticeCon is null", passport);
			return B_ERROR;
		}
		
		// Set up message
		Command *msg = new Command("MSG");
		msg->AddParam("N"); // Don't ack packet
		BString format = "MIME-Version: 1.0\r\n"
			"Content-Type: text/plain; charset=UTF-8\r\n"
			"X-MMS-IM-Format: FN=Arial; EF=I; CO=0; CS=0; PF=22\r\n\r\n";
		format << message;
		
		msg->AddPayload(format.String(), format.Length());
		
		// Find connection
		bool needSB = false;
		connectionlist::iterator it;
		for ( it=fConnections.begin(); it != fConnections.end(); it++ )
		{
			MSNSBConnection * c = dynamic_cast<MSNSBConnection*>( *it );
			
			if ( c != NULL  && c->IsSingleChatWith( passport ) )
				break;
		}
		
		Command *sbReq = NULL;
		
		if (it == fConnections.end()) {
			LOG(kProtocolName, liDebug, "Could not message %s, no open connection: opening new",
				passport);
			
			sbReq =  new Command("XFR");
			sbReq->AddParam("SB");	// Request a SB connection;
			
			fNoticeCon->Send(sbReq, qsImmediate);	
			
			needSB = true;
		};
		
		if (needSB) {
			fWaitingSBs[sbReq->TransactionID()] = std::pair<BString,Command*>(passport, msg);
		} else {
			(*it)->Send(msg);
		};
		
		return B_OK;
	};
	
	LOG(kProtocolName, liHigh, "Error sending message to %s", passport);
	
	return B_ERROR;
};

status_t MSNManager::AddBuddy(const char *buddy) {
	if ( !fNoticeCon )
	{
		LOG(kProtocolName, liDebug, "MSNManager::AddBuddy(%s): invalid fNoticeCon. Aborting!", buddy);	
		
		return B_ERROR;
	}
	
	if( !buddy )
	{
		LOG(kProtocolName, liDebug, "MSNManager::AddBuddy(%s): called with NULL argument. Aborting!", buddy);
		
		return B_ERROR;
	}
	
	LOG(kProtocolName, liDebug, "MSNManager::AddBuddy(%s)", buddy);
	
	// check for duplicates first?
	
	BString n("N="); n += buddy;
	BString f("F=someone"); f += buddy; f.ReplaceAll("@", "%40");
	
	Command * add_fl = new Command("ADC");
	add_fl->AddParam("FL" /* FL, 1 */);
	add_fl->AddParam( n.String() );
	add_fl->AddParam( f.String() );
	
	fNoticeCon->Send(add_fl);
	
	return B_OK;
};

status_t MSNManager::AddBuddies(std::list <char *>buddies) {
/*	for ( std::list<char*>::iterator i=buddies.begin(); i!=buddies.end(); i++ ) {
		// BuddyDetails will create the contact if needed
		BuddyDetails(*i);
	}
*/	
	return B_OK;
};

int32 MSNManager::Buddies(void) const {
	return fBuddy.size();
};

Buddy *MSNManager::BuddyDetails(const char *passport) {
	Buddy *bud = NULL;
	buddymap::iterator bIt = fBuddy.find(passport);
	if (bIt != fBuddy.end()) 
		bud = bIt->second;
	else {
		// new buddy, add it!
		bud = new Buddy(passport);
		fBuddy[passport] = bud;
		// request an add to list here as well, I suppose..
		AddBuddy( passport );
	}
	
	return bud;
};

uchar MSNManager::IsConnected(void) const {
	return fConnectionState;
};

status_t MSNManager::LogOff(void) {
	status_t ret = B_ERROR;

	connectionlist::iterator it;
	
	std::list<thread_id> threads;
	
	for (it = fConnectionPool.begin(); it != fConnectionPool.end(); it++) {
		BMessenger((*it)).SendMessage(B_QUIT_REQUESTED);
	}
	fConnectionPool.clear();
	
	LOG(kProtocolName, liLow, "%i connection(s) to kill", fConnections.size());

	for (it = fConnections.begin(); it != fConnections.end(); it++) {
		MSNConnection *con = (*it);
		
		threads.push_back( con->Thread() );
		
		LOG(kProtocolName, liLow, "Killing switchboard connection %lX", con);
		
		LOG(kProtocolName, liDebug, "  B_QUIT_REQUESTED");
		BMessenger(con).SendMessage(B_QUIT_REQUESTED);
	};
	fConnections.clear();
	
	fConnectionState = otOffline;
	
	if (fNoticeCon) {
		LOG(kProtocolName, liDebug, "Closing fNoticeCon");
		threads.push_back( fNoticeCon->Thread() );
		Command *bye = new Command("OUT");
		bye->UseTrID(false);
		fNoticeCon->Send(bye, qsImmediate);
		
		BMessenger(fNoticeCon).SendMessage(B_QUIT_REQUESTED);
		
		fNoticeCon = NULL;
		LOG(kProtocolName, liDebug, "  Unset fNoticeCon");
	};
	
	if ( fPassport != "" )
		fHandler->StatusChanged(fPassport.String(), otOffline);
	
	wait_for_threads( threads );
	
	ret = B_OK;
	
	return ret;
};

status_t MSNManager::RequestBuddyIcon(const char *buddy) {
	LOG(kProtocolName, liDebug, "Requesting buddy icon for \"%s\"", buddy);
	return B_OK;
};

status_t MSNManager::TypingNotification(const char *passport, uint16 typing) {
	LOG(kProtocolName, liDebug, "Typing notify to %s", passport);
		
	if ((fConnectionState != otOffline) && (fConnectionState != otConnecting)) {
		if (fNoticeCon == NULL) {
			LOG(kProtocolName, liDebug, "Can't send typing notify to %s, fNoticeCon is null", passport);
			return B_ERROR;
		}
		
		// Set up message
		Command *msg = new Command("MSG");
		msg->AddParam("U");
		BString format = "MIME-Version: 1.0\r\n"
			"Content-Type: text/x-msmsgscontrol\r\n"
			"TypingUser: ";
		format << fPassport << "\r\n";
		format << "\r\n\r\n";
		
		msg->AddPayload(format.String(), format.Length());
						
		// Find connection
		bool needSB = false;
		connectionlist::iterator it;
		for ( it=fConnections.begin(); it != fConnections.end(); it++ )
		{
			MSNSBConnection * c = dynamic_cast<MSNSBConnection*>( *it );
			
			if ( c != NULL  && c->IsSingleChatWith( passport ) )
				break;
		}
		
		Command *sbReq = NULL;
		
		if (it == fConnections.end()) {
			LOG(kProtocolName, liDebug, "Can't send typing notify to %s, no open connection: opening new",
				passport);
			
			sbReq =  new Command("XFR");
			sbReq->AddParam("SB");	// Request a SB connection;
			
			fNoticeCon->Send(sbReq, qsImmediate);	
			
			needSB = true;
		};
		
		if (needSB) {
			fWaitingSBs[sbReq->TransactionID()] = std::pair<BString,Command*>(passport, msg);
		} else {
			(*it)->Send(msg);
		};
		
		return B_OK;
	};
	
	LOG(kProtocolName, liHigh, "Error sending typing notification to %s", passport);
	
	return B_ERROR;
};

status_t MSNManager::SetAway(bool away) {
	if (fNoticeCon) {
		Command *awayCom = new Command("CHG");
		
		if (away) {
			awayCom->AddParam("AWY");
			fConnectionState = otAway;
		} else {
			awayCom->AddParam("NLN");
			fConnectionState = otOnline;
		};
		
		BString caps = "";
		caps << kOurCaps;
	
		awayCom->AddParam(caps.String());
	
		fNoticeCon->Send(awayCom);
		
		return B_OK;
	};
	
	return B_ERROR;
};

status_t MSNManager::SetDisplayName(const char *displayname) {
	if (fNoticeCon) {
		fDisplayName = displayname;
		
		Command *rea = new Command("PRP");
		rea->AddParam("MFN");
		rea->AddParam(DisplayName(), true);
	
		fNoticeCon->Send(rea);
		return B_OK;
	};
	
	return B_ERROR;
};

status_t MSNManager::AuthUser(const char *passport) {
	status_t ret = B_ERROR;

	if (fNoticeCon) {
		buddymap::iterator i = fWaitingAuth.find(passport);
		if (i != fWaitingAuth.end()) fWaitingAuth.erase(i);
	
		Command *com = new Command("ADC");
		com->AddParam("AL");	// Allow to see our presence

		BString passParam = passport;
		passParam.Prepend("N=");
		com->AddParam(passParam.String());	// Passport
		
		ret = fNoticeCon->Send(com);
	};
	
	return ret;
};

status_t MSNManager::BlockUser(const char *passport) {
	status_t ret = B_ERROR;

	if (fNoticeCon) {
		buddymap::iterator i = fWaitingAuth.find(passport);
		if (i != fWaitingAuth.end()) fWaitingAuth.erase(i);
	
		Command *com = new Command("ADC");
		com->AddParam("BL");	// Disallow to see our presence

		BString passParam = passport;
		passParam.Prepend("N=");
		com->AddParam(passParam.String());// Passport
		
		ret = fNoticeCon->Send(com);
	};
	
	return ret;
};

void MSNManager::UpdateContactInfo( BMessage * msg ) {
	buddymap::iterator i = fBuddy.find(msg->FindString("passport"));
	
	Buddy * bud = NULL;
	
	// Find or create Buddy
	if ( i == fBuddy.end() ) {
		bud =  new Buddy( msg->FindString("passport") );
		fBuddy[msg->FindString("passport")] = bud;
	} else {
		bud = i->second;
	}
	
	// update info
	if ( msg->FindString("display") )
		bud->FriendlyName( msg->FindString("display") );
	
	int32 lists=0;
	
	if ( msg->FindInt32("lists", &lists) == B_OK ) {
		bud->Lists(lists);
		
		if ( bud->Lists() == ltReverseList ) {
			LOG(kProtocolName, liDebug, "\"%s\" (%s) is only on our reverse list. Likely they "
				"added us while we were offline. Ask for authorisation", bud->FriendlyName(),
				bud->Passport());
			Handler()->AuthRequest(ltReverseList, bud->Passport(), bud->FriendlyName());
		}
		
		if ( bud->Lists() == 0 ) {
			// Buddy in no lists, add to FL
			BString n("N="); n += bud->Passport();
			// should be friendly name, but we'll have to HTML-encode it first!
			BString f("F="); f += bud->Passport(); f.ReplaceAll("@", "%40"); 
			
			Command * add_fl = new Command("ADC");
			add_fl->AddParam("FL" /* FL, 1 */);
			add_fl->AddParam( n.String() );
			add_fl->AddParam( f.String() );
	
			fNoticeCon->Send(add_fl);
		}
	}
}
