#include "MSN.h"
/*
	Written by Michael "slaad" Davidson based off docs from
		http://www.hypothetic.org/docs/msn/
*/

#include <libim/Constants.h>
#include <libim/Helpers.h>

#include <openssl/rand.h>

#include <UTF8.h>

const char *kProtocolName = "msn";
const char *kThreadName = "IMKit: MSN Protocol";

extern "C" {
	IM::Protocol *load_protocol() {	
		return new MSNProtocol();
	};
};

MSNProtocol::MSNProtocol()
	: IM::Protocol( Protocol::MESSAGES | Protocol::SERVER_BUDDY_LIST ),
	  fThread(0) {

	srand(time(NULL));
	
	fPassword = "";
	fPassport = "";
	fDisplayName = "IM Kit User";
	fManager = new MSNManager(dynamic_cast<MSNHandler *>(this));
	
	// OpenSSL random seed
	int random_data[32];
	for (int i = 0; i < 32; i++) {
		random_data[i] = rand();
	};
	RAND_seed(random_data, sizeof(random_data));
};

MSNProtocol::~MSNProtocol() {
}

status_t MSNProtocol::Init(BMessenger msgr) {
	fMsgr = msgr;
	LOG(kProtocolName, liMedium, "MSNProtocol::Init() start");
	
	fManager->Run();
	
	return B_OK;
}

status_t MSNProtocol::Shutdown() {
	thread_id manager_thread_id = fManager->Thread();
	
	fManager->LogOff();
	
	BMessenger(fManager).SendMessage( B_QUIT_REQUESTED );
//	if (fManager->Lock()) fManager->Quit();
	
	fManager = NULL;
	
	int32 res=0;
	
	wait_for_thread( manager_thread_id, &res );
	
	LOG(kProtocolName, liMedium, "MSNProtocol::Shutdown() done");
	
	return B_OK;
}

status_t MSNProtocol::Process(BMessage * msg) {
	switch (msg->what) {
		case IM::MESSAGE: {
			int32 im_what=0;
			
			msg->FindInt32("im_what",&im_what);
		
			switch (im_what) {
				case IM::REGISTER_CONTACTS: {			
					type_code garbage;
					int32 count = 0;
					msg->GetInfo("id", &garbage, &count);
								
					if (count > 0) {
						list<char *> buddies;
						for ( int i=0; msg->FindString("id",i); i++ )
						{
							const char * id = msg->FindString("id",i);
							buddies.push_back(strdup(id));
						};
						fManager->AddBuddies(buddies);
					} else {
						fManager->AddBuddy(msg->FindString("id"));
					};
				}	break;

				case IM::SEND_AUTH_ACK: {
					bool authreply = false;
					
					const char * id = msg->FindString("id");
					int32 button = msg->FindInt32("which");

					if (button == 0) authreply = true;
				
					if (authreply == true) {
						fManager->AuthUser(id);

						// Create a new contact now that we authorized him/her/it.
						BMessage im_msg(IM::MESSAGE);
						im_msg.AddInt32("im_what", IM::CONTACT_AUTHORIZED);
						im_msg.AddString("protocol", kProtocolName);
						im_msg.AddString("id", id);
						im_msg.AddString("message", "" );

						fMsgr.SendMessage(&im_msg);
					} else {
						fManager->BlockUser(id);
					}
				} break;

				case IM::SET_STATUS: {
					const char *status = msg->FindString("status");
					LOG(kProtocolName, liMedium, "Set status to %s", status);
					
					if (strcmp(status, OFFLINE_TEXT) == 0) {
						fManager->LogOff();
					} else
					if (strcmp(status, AWAY_TEXT) == 0) {
						if (fManager->ConnectionState() == (uchar)otOnline) {
							fManager->SetAway(true);
						};
					} else
					if (strcmp(status, ONLINE_TEXT) == 0) {
						if (fManager->IsConnected() == otAway) {
							fManager->SetAway(false);
						} else {
							LOG(kProtocolName, liDebug, "Calling fManager.Login()");
							fManager->Login("messenger.hotmail.com", kDefaultPort,
								fPassport.String(), fPassword.String(),
								fDisplayName.String());
						};
					} else
					{
						LOG(kProtocolName, liHigh, "Invalid status when setting status: '%s'", status);
					}
				} break;
//
//				case IM::GET_CONTACT_INFO:
//				{
//					LOG(kProtocolName, liLow, "Getting contact info");
//					const char * id = NormalizeNick(msg->FindString("id")).String();
//					
//					BMessage *infoMsg = new BMessage(IM::MESSAGE);
//					infoMsg->AddInt32("im_what", IM::CONTACT_INFO);
//					infoMsg->AddString("protocol", kProtocolName);
//					infoMsg->AddString("id", id);
//					infoMsg->AddString("nick", id);
//					infoMsg->AddString("first name", id);
//					//msg->AddString("last name", id);
//
//					fMsgr.SendMessage(infoMsg);
//				}	break;
//		
				case IM::SEND_MESSAGE: {

					const char *message_text = msg->FindString("message");
					const char *passport = msg->FindString("id");
					
					LOG(kProtocolName, liDebug, "SEND_MESSAGE (%s, %s)", msg->FindString("id"), msg->FindString("message"));
					
					if (!passport) return B_ERROR;
					if (!message_text) return B_ERROR;
					
					BString message;
					nl2crlf(message_text, message);
					
					if (fManager->MessageUser(passport, message.String()) == B_OK) {
					
						BMessage newMsg(*msg);
						
						newMsg.RemoveName("contact");
						newMsg.ReplaceInt32("im_what", IM::MESSAGE_SENT);
						
						fMsgr.SendMessage(&newMsg);
					};
					
				}	break;
				case IM::USER_STARTED_TYPING: {
					const char *id = msg->FindString("id");
					if (!id) return B_ERROR;
					
					fManager->TypingNotification(id, 1001 /* should be STARTED_TYPING*/);
				} break;
			}
		}	break;
		default:
			break;
	}
	
	return B_OK;
}

const char * MSNProtocol::GetSignature() {
	return kProtocolName;
}

const char * MSNProtocol::GetFriendlySignature() {
	return "MSN";
}

BMessage MSNProtocol::GetSettingsTemplate() {
	BMessage main_msg(IM::SETTINGS_TEMPLATE);
	
	BMessage user_msg;
	user_msg.AddString("name","passport");
	user_msg.AddString("description", "Passport Email");
	user_msg.AddInt32("type",B_STRING_TYPE);
	
	BMessage pass_msg;
	pass_msg.AddString("name","password");
	pass_msg.AddString("description", "Password");
	pass_msg.AddInt32("type",B_STRING_TYPE);
	pass_msg.AddBool("is_secret", true);
	
	BMessage screen_msg;
	screen_msg.AddString("name", "displayname");
	screen_msg.AddString("description", "Display Name");
	screen_msg.AddInt32("type", B_STRING_TYPE);

	BMessage homePhoneMsg;
	homePhoneMsg.AddString("name", "homephone");
	homePhoneMsg.AddString("description", "Home Phone Number");
	homePhoneMsg.AddInt32("type", B_STRING_TYPE);
	
	BMessage workPhoneMsg;
	workPhoneMsg.AddString("name", "workphone");
	workPhoneMsg.AddString("description", "Work Phone Number");
	workPhoneMsg.AddInt32("type", B_STRING_TYPE);
	
	BMessage mobilePhoneMsg;
	mobilePhoneMsg.AddString("name", "mobilephone");
	mobilePhoneMsg.AddString("description", "Mobile Phone Number");
	mobilePhoneMsg.AddInt32("type", B_STRING_TYPE);
	
	main_msg.AddMessage("setting", &user_msg);
	main_msg.AddMessage("setting", &pass_msg);
	main_msg.AddMessage("setting", &screen_msg);
//	main_msg.AddMessage("setting", &homePhoneMsg);
//	main_msg.AddMessage("setting", &workPhoneMsg);
//	main_msg.AddMessage("setting", &mobilePhoneMsg);
	
	return main_msg;
}

status_t MSNProtocol::UpdateSettings( BMessage & msg ) {
	const char *passport = NULL;
	const char *password = NULL;
	const char *displayname = NULL;
//	const char *encoding = NULL;

	msg.FindString("passport", &passport);
	msg.FindString("password", &password);
	msg.FindString("displayname", &displayname);
	
	if ((passport == NULL) || (password == NULL) ) {
//		invalid settings, fail
		return B_ERROR;
	};
	
	fPassport = passport;
	fPassword = password;
	fDisplayName = displayname;
	
	fManager->SetDisplayName(fDisplayName.String());
	
	return B_OK;
}

uint32 MSNProtocol::GetEncoding() 
{
	return 0xffff; // No conversion, MSN handles UTF-8
}

status_t MSNProtocol::StatusChanged(const char *nick, online_types status) {
	BMessage msg(IM::MESSAGE);
	msg.AddString("protocol", kProtocolName);

	if (fPassport == nick) {
		msg.AddInt32("im_what", IM::STATUS_SET);
	} else {
		msg.AddInt32("im_what", IM::STATUS_CHANGED);
		msg.AddString("id", NormalizeNick(nick));
	};

	switch (status) {
		case otOnline:
		case otIdle: {
			msg.AddString("status", ONLINE_TEXT);
		} break;
		case otAway:
		case otBusy:
		case otBRB:
		case otPhone:
		case otLunch:
		case otInvisible:
		case otBlocked: {
			msg.AddString("status", AWAY_TEXT);
		} break;
		case otOffline: {
			msg.AddString("status", OFFLINE_TEXT);
		} break;
		
		default: {
			return B_ERROR;
		};
	};

	fMsgr.SendMessage(&msg);
	
	return B_OK;
};

status_t MSNProtocol::MessageFromUser(const char *passport, const char *_msg) {
	BString msg;
	crlf2nl(_msg, msg);
	
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::MESSAGE_RECEIVED);
	im_msg.AddString("protocol", kProtocolName);
	im_msg.AddString("id", NormalizeNick(passport));
	im_msg.AddString("message", msg.String());
	
	fMsgr.SendMessage(&im_msg);											
	
	return B_OK;
};

status_t MSNProtocol::UserIsTyping(const char *nick, typing_notification type) {
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddString("protocol", kProtocolName);
	im_msg.AddString("id", NormalizeNick(nick));

	switch (type) {
		case tnStartedTyping: {
			im_msg.AddInt32("im_what", IM::CONTACT_STARTED_TYPING);
		} break;
		case tnStoppedTyping: 
		default: {
			im_msg.AddInt32("im_what", IM::CONTACT_STOPPED_TYPING);
		} break;
	};
	
	fMsgr.SendMessage(&im_msg);
	
	return B_OK;
};

status_t MSNProtocol::SSIBuddies(list<BString> buddies) {
	list <BString>::iterator i;

	BMessage serverBased(IM::SERVER_BASED_CONTACT_LIST);
	serverBased.AddString("protocol", kProtocolName);

	for (i = buddies.begin(); i != buddies.end(); i++) {
		LOG(kProtocolName, liLow, "Got server side buddy %s", NormalizeNick(i->String()).String());
		serverBased.AddString("id", NormalizeNick(i->String()));
	};
			
	fMsgr.SendMessage(&serverBased);
};

BString MSNProtocol::NormalizeNick(const char *nick) {
	BString normal = nick;
	
	normal.ReplaceAll(" ", "");
	normal.ToLower();
	
	map<string,BString>::iterator i = fNickMap.find(normal.String());
	
	if ( i == fNickMap.end() ) {
		// add 'real' nick if it's not already there
		LOG(kProtocolName, liDebug, "Adding normal (%s) vs screen (%s)", normal.String(), nick );
		fNickMap[string(normal.String())] = BString(nick);
	}
	
	LOG(kProtocolName, liDebug, "Screen (%s) to normal (%s)", nick, normal.String() );
	
	return normal;
};

BString MSNProtocol::GetScreenNick( const char *nick ) {
	map<string,BString>::iterator i = fNickMap.find(nick);
	
	if ( i != fNickMap.end() ) {
		// found the nick
		LOG(kProtocolName, liDebug, "Converted normal (%s) to screen (%s)", nick, (*i).second.String() );
		return (*i).second;
	}
	
	LOG(kProtocolName, liDebug, "Nick (%s) not found in fNickMap, not converting", nick );
	
	return BString(nick);
};

status_t MSNProtocol::ContactList(list<BString> *contacts) {
	const char *id;
	BMessage reply;
	BMessage msg(IM::GET_CONTACTS_FOR_PROTOCOL);
	msg.AddString("protocol", kProtocolName);

	fMsgr.SendMessage(&msg, &reply);

	for (int32 i = 0; reply.FindString("id", i, &id) == B_OK; i++) {
		contacts->push_back(id);
	}
	
	return B_OK;
};

status_t MSNProtocol::AuthRequest(list_types /*list*/,const char *passport, const char *displayname) {
	BString reason = displayname;
	reason << " wishes to add you to their list";

	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::AUTH_REQUEST);
	im_msg.AddString("protocol", kProtocolName);
	im_msg.AddString("id", passport);
	im_msg.AddString("message", reason);
	
	fMsgr.SendMessage(&im_msg);
	
	return B_OK;
};

status_t MSNProtocol::Error( const char * error_message ) {
	BMessage msg(IM::ERROR);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("error", error_message);
	
	fMsgr.SendMessage(&msg);
	
	return B_OK;
}

status_t MSNProtocol::Progress( const char * id, const char * message, float progress ) {
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::PROGRESS );
	msg.AddString("protocol", kProtocolName);
	msg.AddString("progressID", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", IM::impsConnecting );
	
	fMsgr.SendMessage(&msg);
	
	return B_OK;
}
