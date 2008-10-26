#include "GoogleTalk.h"
#include <stdio.h>
#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <libjabber/JabberSSLPlug.h>

#include "States.h"
#include "string.h"	

const char *  kProtocolName = "gtalk";

int64 idsms=0;

extern "C" IM::Protocol * load_protocol()
{
	return new GoogleTalk();
}


GoogleTalk::GoogleTalk()
:	IM::Protocol( IM::Protocol::MESSAGES | IM::Protocol::SERVER_BUDDY_LIST | IM::Protocol::OFFLINE_MESSAGES),
	JabberHandler("jabberHandler", fPlug = new JabberSSLPlug("talk.google.com",5223) ),
	fUsername(""),
	fServer("gmail.com"),
	fPassword("")
{

}

GoogleTalk::~GoogleTalk(){
	Shutdown();
}

status_t
GoogleTalk::Init( BMessenger msgr )
{
	fServerMsgr = msgr;
	fAuth=false;
	fRostered=false;
	fAgent=false;
	fFullLogged=false;
	fPerc=0.0;
	fLaterBuddyList=new StrList;
	return B_OK;
	
}

status_t
GoogleTalk::Shutdown()
{

	LogOff();
	fLaterBuddyList->clear();
	delete fLaterBuddyList;
	
	
	thread_id plug = fPlug->Thread();
	BMessenger(fPlug).SendMessage( B_QUIT_REQUESTED );
	fPlug = NULL;
	int32 res=0;
	wait_for_thread( plug, &res );
	
	return B_OK;
}


status_t
GoogleTalk::Process( BMessage * msg )
{
	switch ( msg->what )
	{
		case IM::MESSAGE:
		{
			int32 im_what = 0;
			
			msg->FindInt32("im_what", &im_what );
						
			switch ( im_what )
			{
				case IM::SET_STATUS:
				{
					const char *status = msg->FindString("status");
					LOG(kProtocolName, liMedium, "Set status to %s", status);
					
					if (strcmp(status, OFFLINE_TEXT) == 0) 
					{
							
						SetStatus(S_OFFLINE,OFFLINE_TEXT); //do the log-out?
					} 
					else 
					if (strcmp(status, AWAY_TEXT) == 0) 
					{
						if(IsAuthorized()){
						
						 //const char *away_msg;	
						 BString away_msg;					 
						 if(msg->FindString("away_msg",&away_msg) == B_OK)
						 {
						 	// quick and dirty way to use advanced away status:
						 	// add 'DND: ' for Do not Disturb
						 	// or  'XA: ' for Extended Away
						 	
						 	if(away_msg.Compare("DND: ",4) == 0)
						 		SetStatus(S_DND,away_msg); 
						 	else
						 	if(away_msg.Compare("XA: ",4) == 0)
						 		SetStatus(S_XA,away_msg); 
						 	else	
						 		SetStatus(S_AWAY,away_msg); 
						 }
						 	 else
						 		 SetStatus(S_AWAY,AWAY_TEXT); 
						 
						 			
						 SetAway(true);
						} 
					} 
					else 
					if (strcmp(status, ONLINE_TEXT) == 0) 
					{
							if(!IsAuthorized())
							{
								if(fUsername == "")
									Error("Empty Username!",NULL);
								if(fServer == "")
									Error("Empty Server!",NULL);
								if(fPassword == "")
									Error("Empty Password!",NULL);
								
								Progress("GoogleTalk Login", "GoogleTalk: Connecting..", 0.0);
										
							}
							
							SetStatus(S_ONLINE,ONLINE_TEXT); //do the login!
							if(IsAuthorized()) SetAway(false); 
					} 
					else
					{
						Error("Invalid",NULL);
						LOG(kProtocolName, liHigh, "Invalid status when setting status: '%s'", status);
					}
				}	break;
				
				case IM::SEND_MESSAGE:
				{
						const char * buddy=msg->FindString("id");
						const char * sms=msg->FindString("message");
						JabberMessage jm;
						jm.SetTo(buddy);
						jm.SetFrom(GetJid());
						jm.SetBody(sms);
						TimeStamp(jm);
						
						//not the right place.. see Jabber::Message
						JabberContact *contact=getContact(buddy);
						
						//tmp: new mess id!
						BString messid("imkit_");
						messid << idsms;
						idsms++;
						
						if(contact)
							jm.SetID(messid);
												
						SendMessage(jm);
						
						MessageSent(buddy,sms);
				} 
				break;
				case IM::REGISTER_CONTACTS:
					
					{
					
					//debugger("REGISTER");
					type_code garbage;
					int32 count = 0;
					msg->GetInfo("id", &garbage, &count);
					
										
					if (count > 0 ) {
						
						for ( int i=0; msg->FindString("id",i); i++ )
						{
							const char * id = msg->FindString("id",i);
							JabberContact* contact=getContact(id);
							if(contact)
								  BuddyStatusChanged(contact);
							else
							{
							 
								//Are we on-line?
								// send auth req?
								if(fFullLogged)
								{ 			
									AddContact(id,id,"");
									BuddyStatusChanged(id,OFFLINE_TEXT);
								}
								
								else
								{
								 // we add to a temp list.
								 // when logged in we will register the new buddy..
									 fLaterBuddyList->push_back(BString(id));
								}							 
							} 
						};
						
					} 
					else 
						return B_ERROR;
					} 	
					break;
				case IM::UNREGISTER_CONTACTS:
				
				{
						
					
						const char * buddy=NULL;
						
						for ( int i=0; msg->FindString("id", i, &buddy) == B_OK; i++ )
						{
							LOG(kProtocolName, liDebug, "Unregister Contact: '%s'", buddy);
							
							if(!fFullLogged)
							BuddyStatusChanged(buddy,OFFLINE_TEXT);
							else
							{
								LOG(kProtocolName, liDebug, "Unregister Contact DOING IT");
							 	JabberContact* contact=getContact(buddy);
							 	if(contact)
									RemoveContact(contact);
							}
						}
				} 
					
					break;
				case IM::USER_STARTED_TYPING: 
				{
						const char * id=NULL;
						
						if( msg->FindString("id", &id) == B_OK )
						{
						 JabberContact* contact=getContact(id);
						 if(contact)
							StartComposingMessage(contact);
						}
				} 
				break;
				case IM::USER_STOPPED_TYPING: 
				{
						const char * id=NULL;
						
						if( msg->FindString("id", &id) == B_OK )
						{
						 JabberContact* contact=getContact(id);
						 if(contact && (contact->GetLastMessageID().ICompare("")!=0)){
							StopComposingMessage(contact);
							contact->SetLastMessageID("");
							
							}
						}
				} 
				break;
				
				case IM::GET_CONTACT_INFO:
					//debugger("Get Contact Info! ;)");
					SendContactInfo(msg->FindString("id"));
				break;
								
				case IM::SEND_AUTH_ACK:
				{
					if(!IsAuthorized())
						return B_ERROR;
					
					const char * id = msg->FindString("id");
					int32 button = msg->FindInt32("which");
					
					if (button == 0) {
						
						//Authorization granted
						AcceptSubscription(id);
						BMessage im_msg(IM::MESSAGE);
						im_msg.AddInt32("im_what", IM::CONTACT_AUTHORIZED);
						im_msg.AddString("protocol", kProtocolName);
						im_msg.AddString("id", id);
						im_msg.AddString("message", "");
						fServerMsgr.SendMessage(&im_msg);
						
						//now we want to see you! ;)
						AddContact(id,id,"");
														
					} 
					else 
					{
						//Authorization rejected
						Error("Authorization rejected!",id);
					}
						
					
				}
				break;
				case IM::SPECIAL_TO_PROTOCOL:
				{
						/*BMessage im_msg(IM::MESSAGE);
						im_msg.AddInt32("im_what", IM::SPECIAL_FROM_PROTOCOL);
						im_msg.AddString("protocol", kProtocolName);
						im_msg.AddString("uid_string", "jabber!");
						fServerMsgr.SendMessage(&im_msg);
						*/
						Send(msg->FindString("direct_data"));		
				}
				break;
				default:
					// we don't handle this im_what code
					msg->PrintToStream();	
					return B_ERROR;
			}
		}	break;
		
		default:
			// we don't handle this what code
			return B_ERROR;
	}
	
	return B_OK;
}

const char *
GoogleTalk::GetSignature()
{
	return kProtocolName;
}

const char *
GoogleTalk::GetFriendlySignature()
{
	return "Google Talk";
}





BMessage
GoogleTalk::GetSettingsTemplate()
{
	BMessage main_msg(IM::SETTINGS_TEMPLATE);
	
	BMessage user_msg;
	user_msg.AddString("name","username");
	user_msg.AddString("description", "Gmail Username"); 
	user_msg.AddInt32("type",B_STRING_TYPE);
	/*
	BMessage serv_msg;
	serv_msg.AddString("name","server");
	serv_msg.AddString("description", "Google Server"); 
	serv_msg.AddInt32("type",B_STRING_TYPE);
	*/
	BMessage pass_msg;
	pass_msg.AddString("name","password");
	pass_msg.AddString("description", "Gmail Password");
	pass_msg.AddInt32("type",B_STRING_TYPE);
	pass_msg.AddBool("is_secret", true);
	
	BMessage res;
	res.AddString("name","resource");
	res.AddString("description", "Resource");
	res.AddString("default", "IM Kit GoogleTalk AddOn");
	res.AddInt32("type",B_STRING_TYPE);
		
	main_msg.AddMessage("setting", &user_msg);
	//main_msg.AddMessage("setting", &serv_msg);
	main_msg.AddMessage("setting", &pass_msg);
	main_msg.AddMessage("setting", &res);
	
	return main_msg;
}

status_t
GoogleTalk::UpdateSettings( BMessage & msg )
{
	const char *username = NULL;
	const char *password = NULL;
	const char *res = NULL;
	
	msg.FindString("username", &username);
	msg.FindString("password", &password);
	msg.FindString("resource", &res);
	
	if ( (username == NULL) || (password == NULL) ){ 
		LOG( kProtocolName, liHigh, "Invalid settings!");
		return B_ERROR;
	};
	
	fUsername = username;
	int32 atpos=fUsername.FindLast("@");
	if(	 atpos> 0 ) {
		BString server;
		fUsername.CopyInto(server,atpos + 1,fUsername.Length()-atpos);
		fUsername.Remove(atpos,fUsername.Length()-atpos);
		LOG( kProtocolName, liHigh, "Server %s\n",server.String());
		fServer = server;		
	}
	else
		fServer.SetTo("gmail.com");
		
	
	fPassword = password;
	
	SetUsername(fUsername);
	SetHost(fServer);
	SetPassword(fPassword);
	
	if(strlen(res)==0)
		SetResource("IMKit GoogleTalk AddOn");
	else
		SetResource(res);
	
	SetPriority(5);
	SetPort(5222);
	
	return B_OK;
}

uint32
GoogleTalk::GetEncoding()
{
	return 0xffff; // No conversion, GoogleTalk handles UTF-8 ???
}

// JabberManager stuff

void
GoogleTalk::Error( const char * message, const char * who )
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::Error(%s,%s)", message, who);
	
	BMessage msg(IM::ERROR);
	msg.AddString("protocol", kProtocolName);
	if ( who )
		msg.AddString("id", who);
	msg.AddString("error", message);
	
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::GotMessage( const char * from, const char * message )
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::GotMessage()");
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::MESSAGE_RECEIVED);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("id", from);
	msg.AddString("message", message);
	
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::MessageSent( const char * to, const char * message )
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::GotMessage()");
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::MESSAGE_SENT);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("id", to);
	msg.AddString("message", message);
	
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::LoggedIn()
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::LoggedIn()");
	
	Progress("GoogleTalk Login", "GoogleTalk: Logged in!", 1.00);
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::STATUS_SET);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("status", ONLINE_TEXT);
	
	fServerMsgr.SendMessage( &msg );
	
	fFullLogged=true;
	
	LOG("GoogleTalk", liDebug, "Starting fLaterBuddyList");
	
	while (fLaterBuddyList->size() != 0)
	{
		BString id = *(fLaterBuddyList->begin());
		fLaterBuddyList->pop_front();	// removes first item
		JabberContact* contact=getContact(id.String());
		if(!contact)
		{
			AddContact(id.String(),id.String(),"");
			BuddyStatusChanged(id.String(),OFFLINE_TEXT);
		}	
	}
	
	LOG("GoogleTalk", liDebug, "Ending fLaterBuddyList");
}

void
GoogleTalk::SetAway(bool away)
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::SetAway()");

	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::STATUS_SET);
	msg.AddString("protocol", kProtocolName);
	if ( away )
		msg.AddString("status", AWAY_TEXT);
	else
		msg.AddString("status", ONLINE_TEXT);
	
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::LoggedOut()
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::LoggedOut()");
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::STATUS_SET);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("status", OFFLINE_TEXT);
	fServerMsgr.SendMessage( &msg );
	fFullLogged=false;
	fAuth=false;
	fRostered=false;
	fAgent=false;
	fPerc=0.0;
}

void
GoogleTalk::BuddyStatusChanged( JabberContact* who ){
	BuddyStatusChanged(who->GetPresence());
}

void
GoogleTalk::BuddyStatusChanged( JabberPresence* jp ){
	
	LOG("GoogleTalk", liDebug, "GoogleTalk::BuddyStatusChanged(%s)",jp->GetJid().String());
	
	//avoid a receiving self status changes or empty status:
	if(jp->GetJid() == "" || jp->GetJid().ICompare(GetJid())==0) return;
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::STATUS_CHANGED);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("id", jp->GetJid());
	
	AddStatusString(jp,&msg);
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::AddStatusString(JabberPresence* jp ,BMessage* msg)
{	
	
	int32 show=jp->GetShow();
	switch(show) 
	{
		case S_XA:
			msg->AddString("status", AWAY_TEXT);
			break;
		case S_AWAY:
			msg->AddString("status",AWAY_TEXT );
			break;
		case S_ONLINE:
			msg->AddString("status",ONLINE_TEXT);
			break;
		case S_DND:
			msg->AddString("status", AWAY_TEXT);
			break;
		case S_CHAT:
			msg->AddString("status", ONLINE_TEXT);
			break;
		case S_SEND: //???
			msg->AddString("status", ONLINE_TEXT);
			break;
		default:
		{
			BString test("StatusString");
			test << " [" << jp->GetShow() << "]";
			//debugger(test.String());
			printf("SSLPlug %s",test.String());
			msg->AddString("status", OFFLINE_TEXT);
		}
		break;
	}
}
void
GoogleTalk::BuddyStatusChanged( const char * who, const char * status )
{
	LOG("GoogleTalk", liDebug, "GoogleTalk::BuddyStatusChanged(%s,%s)",who,status);
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::STATUS_CHANGED);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("id", who);
	msg.AddString("status", status);
	
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::Progress( const char * id, const char * message, float progress )
{
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::PROGRESS );
	msg.AddString("protocol", kProtocolName);
	msg.AddString("progressID", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", IM::impsConnecting );
	
	fServerMsgr.SendMessage(&msg);
}

JabberContact*
GoogleTalk::getContact(const char* id)
{
	RosterList *rl=getRosterList();
	JabberContact* contact=NULL;
	//LOG(kProtocolName, liDebug, "getContact %s", id);
	
	for(int i=0;i<rl->CountItems();i++)
	{
		contact=reinterpret_cast<JabberContact*>(getRosterList()->ItemAt(i));
		//LOG(kProtocolName, liDebug, "getContact [%3d] GetJID %s", i,contact->GetJid().String());
		
		if(contact->GetJid().ICompare(id)==0)
		{
			//LOG(kProtocolName, liDebug, "getContact found!");
			return contact;								
		}								
	}
	return NULL;
}

void			
GoogleTalk::SendContactInfo(const char* id)
{
	JabberContact *jid=getContact(id);
	if(jid)
	{
		BMessage msg(IM::MESSAGE);
		msg.AddInt32("im_what", IM::CONTACT_INFO);
		msg.AddString("protocol", kProtocolName);
		msg.AddString("id", id);
		msg.AddString("nick", jid->GetName());     //just nick ??
		fServerMsgr.SendMessage(&msg);	
	}
}
//CALLBACK!
void
GoogleTalk::Authorized()
{
	SetAway(false);
	
	fPerc +=0.3333;
	fAuth=true;
	Progress("GoogleTalk Login", "GoogleTalk: Authorized", fPerc);
	LOG(kProtocolName, liDebug, "GoogleTalk:Login %f - Authorized",fPerc) ;
	CheckLoginStatus();
	
	JabberHandler::Authorized();
}

void
GoogleTalk::Message(JabberMessage * message){
	
	if(message->GetBody()!="") //we have something to tell
		GotMessage(message->GetFrom().String(),message->GetBody().String());
	
	if(message->GetError()!="") //not a nice situation..
	{
		Error(message->GetError().String(),message->GetFrom().String());
		return;
	}
	
	LOG(kProtocolName, liHigh, "GETX: '%s'",message->GetX().String()) ;
				
		
	if(message->GetX().ICompare("composing") == 0)
	{
		//someone send a composing event.
		
		if(message->GetBody() == "") //Notification
		{
		 LOG(kProtocolName, liHigh,"CONTACT_STARTED_TYPING");
		 BMessage im_msg(IM::MESSAGE);
		 im_msg.AddInt32("im_what", IM::CONTACT_STARTED_TYPING);
		 im_msg.AddString("protocol", kProtocolName);
		 im_msg.AddString("id", message->GetFrom());
		 fServerMsgr.SendMessage(&im_msg);	
		}
		else //Request
		{
			//	where we put the last messge id? on the contact (is it the right place?)
			//	maybe we should make an hash table? a BMesage..
			
			JabberContact *contact=getContact(message->GetFrom().String());
			if(contact)
			contact->SetLastMessageID(message->GetID());
		}
	}
	else
	if(message->GetX().ICompare("jabber:x:event") == 0)
	{
		//not define event this maybe due to:
		// 	unkown event.
		// 	no event (means stop all)
		
		LOG(kProtocolName, liHigh,"CONTACT_STOPPED_TYPING");
			
		BMessage im_msg(IM::MESSAGE);
		im_msg.AddInt32("im_what", IM::CONTACT_STOPPED_TYPING);
		im_msg.AddString("protocol", kProtocolName);
		im_msg.AddString("id", message->GetFrom());
		fServerMsgr.SendMessage(&im_msg);
	}	
	

	
}
void
GoogleTalk::Presence(JabberPresence * presence){
	//printf("JabberPresence\nType: %s\nStatus: %s\nShow: %ld\n",presence->GetType().String(),presence->GetStatus().String(),presence->GetShow());
	BuddyStatusChanged(presence);
}

void
GoogleTalk::Roster(RosterList * roster){

	// Fix me! (Roster message can arrive at different times)
	
	BMessage serverBased(IM::SERVER_BASED_CONTACT_LIST);
	serverBased.AddString("protocol", kProtocolName);
	JabberContact* contact;
	int size = roster->CountItems();
	
	for(int i=0; i < size; i++) {
		contact = reinterpret_cast<JabberContact*>(roster->ItemAt(i));
		serverBased.AddString("id", contact->GetJid());
	}	
	fServerMsgr.SendMessage(&serverBased);		
	
	//fRostered=true;
	if(!fRostered){ //here the case when more than one roster message has arrived!
		fPerc +=0.3333;
		fRostered = true;
		Progress("GoogleTalk Login", "GoogleTalk: Roster", fPerc);
	}
	
	LOG(kProtocolName, liDebug, "GoogleTalk:Login %f - Rostered",fPerc) ;
	CheckLoginStatus();
	
}

void
GoogleTalk::Agents(AgentList * agents){
	fPerc +=0.3333;
	fAgent = true;
	Progress("GoogleTalk Login", "GoogleTalk: Agents", fPerc);
	LOG(kProtocolName, liDebug, "GoogleTalk:Login %f - Agents",fPerc) ;
	CheckLoginStatus();
}

void
GoogleTalk::Disconnected(const BString & reason){


	LoggedOut();
	
	if(reason == "") return; // what else should I say?
	
	Error(reason.String(),NULL);
	
	
	
}
void
GoogleTalk::SubscriptionRequest(JabberPresence * presence){
	
	// someone want us!
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::AUTH_REQUEST);
	im_msg.AddString("protocol", kProtocolName);
	im_msg.AddString("id", presence->GetJid());
	im_msg.AddString("message", presence->GetStatus());
		
	fServerMsgr.SendMessage(&im_msg);	
}

void
GoogleTalk::Unsubscribe(JabberPresence * presence){
	
	// what should we do when a people unsubscrive from us?
	//debugger("Unsubscribe");
	LOG("GoogleTalk", liDebug, "GoogleTalk::Unsubscribe()");
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::STATUS_CHANGED);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("id", presence->GetJid());
	msg.AddString("status", OFFLINE_TEXT);
	fServerMsgr.SendMessage( &msg );
}

void
GoogleTalk::Registration(JabberRegistration * registration){
	
	// Just created a new account ?
	// or we have ack of a registration? ack of registartion!
	debugger("Registration");
	registration->PrintToStream();
}

void
GoogleTalk::CheckLoginStatus()
{
	if(fAuth && fRostered &&  fAgent && !fFullLogged) 
		LoggedIn();
		
}

