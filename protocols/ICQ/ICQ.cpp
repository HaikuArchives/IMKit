/*
 * shell.cpp - an example of a simple command line client that uses
 * the ickle ICQ2000 libraries to automatically respond to messages
 * and sms's.
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Examples of usage:
 *  ./ickle-shell uin pass cat
 *   will echo back every message sent to it
 *
 *  ./ickle-shell uin pass fortune
 *   will reply with a fortune cookie :-)
 *
 *
 * This example uses two helper classes:
 *
 * - PipeExec: a class that handles forking a shell command, feeding
 * it input, collected it's output, and terminating/killing it. This
 * class has no particular relevance to libicq2000, and it just here
 * for the coolness of the example. :-)
 *
 * - Select: this class wraps the select() system call up in a nice
 * interface so we don't have to worry about file descriptor lists,
 * etc.. we just register callbacks with it and pass these back to the
 * library. Often the library you use for your user interface will
 * handle this for you too - ie. ncurses, gtk+ or gtkmm. If you are
 * building you own interface from scratch, then this class might come
 * in handy, with a few extensions..
 *
 */

/* Modified to work as a protcol add-on for the BeOS Instant Messaging Kit
 * by Mikael Eiman <mikael@eiman.tv>
 */

#include "ICQ.h"

#include <libim/Constants.h>
#include <libim/Helpers.h>

#include <UTF8.h>

bool g_is_quiting = false;

const char * kProtocolName = "icq";

// ------------------------------------------------------------------
//  Simple Client
// ------------------------------------------------------------------

SimpleClient::SimpleClient(unsigned int uin, const string& pass)
: icqclient(uin, pass),
	fEncoding(0xffff),
	fAwayMessage("away")
{

  /*
   * set up the libicq2000 callbacks: the SigC callback system is used
   * extensively in libicq2000, and when an event happens we can
   * register callbacks for methods to be called, which in turn will
   * be called when the relevant event happens
   */
  icqclient.connected.connect(slot(this,&SimpleClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&SimpleClient::disconnected_cb));
  icqclient.messaged.connect(slot(this,&SimpleClient::message_cb));
  icqclient.logger.connect(slot(this,&SimpleClient::logger_cb));
  icqclient.contact_status_change_signal.connect(slot(this,&SimpleClient::contact_status_change_cb));
  icqclient.self_contact_status_change_signal.connect(slot(this,&SimpleClient::self_status_change_cb));
  icqclient.socket.connect(slot(this,&SimpleClient::socket_cb));
  icqclient.contact_userinfo_change_signal.connect(slot(this,&SimpleClient::contact_userinfo_change_cb));
  icqclient.server_based_contact_list.connect(slot(this,&SimpleClient::server_based_contact_list_cb));
//  icqclient.want_auto_resp.connect(slot(this,&SimpleClient::want_auto_resp_cb)); Disabled to see if it stops the ICQ crashes.
}

SimpleClient::SimpleClient()
:	fEncoding(0xffff),
	fAwayMessage("away")
{
  /*
   * set up the libicq2000 callbacks: the SigC callback system is used
   * extensively in libicq2000, and when an event happens we can
   * register callbacks for methods to be called, which in turn will
   * be called when the relevant event happens
   */
  icqclient.connected.connect(slot(this,&SimpleClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&SimpleClient::disconnected_cb));
  icqclient.messaged.connect(slot(this,&SimpleClient::message_cb));
  icqclient.logger.connect(slot(this,&SimpleClient::logger_cb));
  icqclient.contact_status_change_signal.connect(slot(this,&SimpleClient::contact_status_change_cb));
  icqclient.self_contact_status_change_signal.connect(slot(this,&SimpleClient::self_status_change_cb));
  icqclient.socket.connect(slot(this,&SimpleClient::socket_cb));
  icqclient.contact_userinfo_change_signal.connect(slot(this,&SimpleClient::contact_userinfo_change_cb));
  icqclient.server_based_contact_list.connect(slot(this,&SimpleClient::server_based_contact_list_cb));
//  icqclient.want_auto_resp.connect(slot(this,&SimpleClient::want_auto_resp_cb)); Disabled to see if it stops the ICQ crashes.
}

void SimpleClient::run() {

  icqclient.setStatus(STATUS_ONLINE);
  
  g_is_quiting = false;
  
  while ( !g_is_quiting ) {
    /*
     * the input object (a Select object, written to wrap a nicer C++
     * wrapper round the C select(2) system call), handles the lists
     * of file descriptors we need to select on for read or write
     * (multiplex).
     */
    bool b = input.run(5000);

    if (b) icqclient.Poll();
    // timeout was hit - poll the server
  }
  
  // never reached
  icqclient.setStatus(STATUS_OFFLINE);
}

/*
 * this callback will be called when the library has a socket
 * descriptor that needs select'ing on (or not selecting on any
 * longer), we pass it up to the higher-level Select object which
 * wraps all the select system call stuff nicely -- a graphical
 * toolkit might be the other way to handle this for you (for
 * example the way gtkmm does it)
 */
void SimpleClient::socket_cb(SocketEvent *ev) {
  
  if (dynamic_cast<AddSocketHandleEvent*>(ev) != NULL) {
    // the library requests we start selecting on a socket

    AddSocketHandleEvent *cev = dynamic_cast<AddSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();
	
	LOG("icq", liLow, "ICQ: connecting socket %ld", fd);

    // register this socket with our Select object
    m_sockets[fd] =
    input.connect( slot(this,&SimpleClient::select_socket_cb),
		   // the slot that the Select object will callback
		   fd,
		   // the socket file descriptor to add
		   (Select::SocketInputCondition) 
		   ((cev->isRead() ? Select::Read : 0) |
		    (cev->isWrite() ? Select::Write : 0) |
		    (cev->isException() ? Select::Exception : 0))
		   // the mode to select on it on
		   );

  } else if (dynamic_cast<RemoveSocketHandleEvent*>(ev) != NULL) {
    // the library requests we stop selecting on a socket

    RemoveSocketHandleEvent *cev = dynamic_cast<RemoveSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    LOG("icq", liLow, "ICQ: disconnecting socket %ld", fd);
    
    if (m_sockets.count(fd) == 0) {
      LOG("icq", liHigh, "ICQ: Problem: file descriptor not connected");
    } else {
      m_sockets[fd].disconnect();
      m_sockets.erase(fd);
    }

  } else
  {
  	LOG("icq", liHigh, "ICQ: Some other socket event that's not handled\n");
  	exit_thread(1);
  }
}

/*
 * registered to receive the Select callbacks
 */
void SimpleClient::select_socket_cb(int fd, Select::SocketInputCondition cond)
{
  // inform the library (it is always only library sockets registered with
  // the Select object)
  icqclient.socket_cb(fd,
		      (ICQ2000::SocketEvent::Mode)cond
		      // dirty-hack, since they are binary compatible :-)
		      );
}

/*
 * called when the library has connected
 */
void SimpleClient::connected_cb(ConnectedEvent */*c*/) {
  LOG("icq", liHigh, "Connected");
  
  icqclient.fetchServerBasedContactList();
  
  BMessage msg(IM::MESSAGE);
  msg.AddInt32("im_what", IM::STATUS_SET);
  msg.AddString("protocol", "icq");
  msg.AddString("status", ONLINE_TEXT);
  fMsgr.SendMessage(&msg);

  Progress( fMsgr, "ICQ: Online!", 1.00 );
}

/*
 * this callback is called when the library needs to indicated
 * connecting failed or we've been disconnected for whatever
 * reason.
 */
void SimpleClient::disconnected_cb(DisconnectedEvent *c) {
  if (c->getReason() == DisconnectedEvent::REQUESTED) {
    LOG("icq", liMedium, "Disconnected as requested");
  } else {
    switch(c->getReason()) {
    case DisconnectedEvent::FAILED_LOWLEVEL: {
      LOG("icq", liHigh, "Problem connecting: socket problems");

	  BMessage msg(IM::ERROR);
	  msg.AddString("protocol", kProtocolName);
	  msg.AddString("error", "Problem connecting: socket problems");
	  fMsgr.SendMessage(&msg);
  
    }  break;
    case DisconnectedEvent::FAILED_BADUSERNAME:
      LOG("icq", liHigh, "Problem connecting: Bad Username");
      break;
    case DisconnectedEvent::FAILED_TURBOING:
      LOG("icq", liHigh, "Problem connecting: Turboing");
      break;
    case DisconnectedEvent::FAILED_BADPASSWORD:
      LOG("icq", liHigh, "Problem connecting: Bad Password");
      break;
    case DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      LOG("icq", liHigh, "Problem connecting: Username and Password did not match");
      break;
    case DisconnectedEvent::FAILED_UNKNOWN:
      LOG("icq", liHigh, "Problem connecting: Unknown");
      break;
    default:
      break;
    }
  }

  BMessage msg(IM::MESSAGE);
  msg.AddInt32("im_what", IM::STATUS_SET);
  msg.AddString("protocol", "icq");
  msg.AddString("status", OFFLINE_TEXT);
  fMsgr.SendMessage(&msg);
  
  g_is_quiting = true;
}

/*
 * this callback is called when someone messages you. Put the
 * message in a BMessage and send it.
 */
void SimpleClient::message_cb(MessageEvent *c) {

  if (c->getType() == MessageEvent::Normal) {

    NormalMessageEvent *msg = static_cast<NormalMessageEvent*>(c);
	LOG("icq", liMedium, "Message received: %s from %ld", msg->getMessage().c_str(), msg->getSenderUIN());
	
	char uin_string[100];
	
	sprintf(uin_string,"%d",msg->getSenderUIN());
	
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::MESSAGE_RECEIVED);
	im_msg.AddString("protocol", "icq");
	im_msg.AddString("id", uin_string);
	im_msg.AddString("message", msg->getMessage().c_str() );
	im_msg.AddInt32("charset",fEncoding);
	
	fMsgr.SendMessage(&im_msg);
  } else if (c->getType() == MessageEvent::AuthReq) {
  	AuthReqEvent *msg = static_cast<AuthReqEvent*>(c);
  	LOG("icq", liMedium, "Authorization request received: %s from %ld", msg->getMessage().c_str(), msg->getSenderUIN());
  	
  	char uin_string[100];
  	
  	sprintf(uin_string,"%d",msg->getSenderUIN());

	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::AUTH_REQUEST);
	im_msg.AddString("protocol", "icq");
	im_msg.AddString("id", uin_string);
	im_msg.AddString("message", msg->getMessage().c_str() );
	im_msg.AddInt32("charset",fEncoding);
	
	fMsgr.SendMessage(&im_msg);	  
  }
}

/*
 * this callback is for debug information the library logs - it is
 * generally very useful, although not immediately useful for any
 * users of your program. It allows great flexibility, as the Client
 * decides where the messages go and some of the formatting of them -
 * so they could be displayed in a Dialog, for example. Here they're
 * dumped out to stdout, with some pretty colours showing the level.
 */
void SimpleClient::logger_cb(LogEvent */*c*/) {
}

/*
	Contact list event
*/
void SimpleClient::contact_userinfo_change_cb( UserInfoChangeEvent * e )
{
	ICQ2000::ContactRef contact = e->getContact();
	
	BMessage * msg = new BMessage(IM::MESSAGE);
	msg->AddInt32("im_what", IM::CONTACT_INFO);
	msg->AddString("protocol", "icq");
	msg->AddInt32("charset", fEncoding );
	msg->AddString("id", contact->getStringUIN().c_str() );
	
	if ( contact->getFirstName().length() > 0 )
		msg->AddString("first name", contact->getFirstName().c_str() );
	
	if ( contact->getLastName().length() > 0 )
		msg->AddString("last name", contact->getLastName().c_str() );
	
	if ( contact->getEmail().length() > 0 )
		msg->AddString("email", contact->getEmail().c_str() );
	
	if ( contact->getAlias().length() > 0 )
	{
		if ( contact->getAlias() != contact->getStringUIN() )
		{ // test to see if alias == UIN and skip if it is
			msg->AddString("nick", contact->getAlias().c_str() );
		}
	}
	
	fMsgr.SendMessage( msg );
	delete msg;
}

/*
 * this callback is called when a contact on your list changes their
 * status
 */
void SimpleClient::contact_status_change_cb(StatusChangeEvent *ev)
{
	char uin_string[256];
	
	sprintf( uin_string,"%d",ev->getUIN() );
	
	const char * status;
	
	switch(ev->getStatus()) 
	{
		case STATUS_ONLINE:
		case STATUS_FREEFORCHAT:
			status = ONLINE_TEXT;
			break;
		case STATUS_NA:
		case STATUS_DND:
		case STATUS_OCCUPIED:
		case STATUS_AWAY:
			status = AWAY_TEXT;
			break;
		case STATUS_OFFLINE:
			status = OFFLINE_TEXT;
			break;
		default:
			status = OFFLINE_TEXT;
			break;
	}
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what",IM::STATUS_CHANGED);
	msg.AddString("protocol","icq");
	msg.AddString("id",uin_string);
	msg.AddString("status",status);
	fMsgr.SendMessage( &msg );
}

/*
 * this callback is called when your online status is changed
 */
void SimpleClient::self_status_change_cb(StatusChangeEvent *ev)
{
	char uin_string[256];
	
	sprintf( uin_string,"%d",ev->getUIN() );
	
	const char * status = NULL;
	
	switch(ev->getStatus()) 
	{
		case STATUS_ONLINE:
		case STATUS_FREEFORCHAT:
			status = ONLINE_TEXT;
			break;
		case STATUS_NA:
		case STATUS_DND:
		case STATUS_OCCUPIED:
		case STATUS_AWAY:
			status = AWAY_TEXT;
			break;
		case STATUS_OFFLINE:
			status = OFFLINE_TEXT;
			break;
		default:
			status = OFFLINE_TEXT;
			break;
	}
	
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what",IM::STATUS_SET);
	msg.AddString("protocol","icq");
	msg.AddString("status",status);
	fMsgr.SendMessage( &msg );
	
	LOG("icq", liLow, "Self status changed to %s", status);
}

/**
*/
void
SimpleClient::server_based_contact_list_cb( ServerBasedContactEvent * ev )
{
	ContactList & list = ev->getContactList();
	
	ContactList::iterator i(list.begin());
	
	BMessage msg( IM::SERVER_BASED_CONTACT_LIST );
	msg.AddString("protocol","icq");
	
	bool addedContact = false;
	
	for ( ; i != list.end(); i++ )
	{
		if ( (*i)->getUIN() )
		{
			msg.AddString("id", (*i)->getStringUIN().c_str());
			icqclient.addContact( *i );
			snooze(10000);
			addedContact = true;
		}
	}
	
	if ( addedContact )
		fMsgr.SendMessage( &msg );
}

/* Disabled to see if it stops the ICQ crashes.
void
SimpleClient::want_auto_resp_cb( ICQMessageEvent * ev )
{
	ev->setAwayMessage( fAwayMessage );
}
*/

void
SimpleClient::setMessenger( BMessenger msgr )
{
	fMsgr = msgr;
}

void
SimpleClient::setEncoding( int32 encoding )
{
	fEncoding = encoding;
}

void
SimpleClient::setAwayMessage( string msg )
{
	fAwayMessage = msg;
}

int32
client_thread( void * _data )
{
	LOG("icq", liLow, "client thread running");

	SimpleClient * client = (SimpleClient*)_data;
	
	client->run();
	
	// we'll never get here.
	return 0;
}


/// im_kit add-on class

extern "C"
IM::Protocol * load_protocol()
{
	return new ICQProtocol();
}

ICQProtocol::ICQProtocol()
:	Protocol( Protocol::MESSAGES | Protocol::OFFLINE_MESSAGES | Protocol::SERVER_BUDDY_LIST ),
	fThread(0)
{
}

ICQProtocol::~ICQProtocol()
{
}

status_t
ICQProtocol::Init( BMessenger msgr )
{
	fMsgr = msgr;
	fClient.setMessenger( msgr );
	
	return B_OK;
}

status_t
ICQProtocol::Shutdown()
{
	fClient.icqclient.setStatus(STATUS_OFFLINE);
	
	while ( fClient.icqclient.isConnected() )
	{ // busy-wait until disconnected
		snooze(1000*1000);
	}
	
	kill_thread( find_thread(ICQ_THREAD_NAME) );
	
	LOG("icq", liMedium, "ICQProtocol::Shutdown() done");
	
	return B_OK;
}

status_t
ICQProtocol::Process( BMessage * msg )
{
//	printf("ICQProtocol::Process()\n");
//	msg->PrintToStream();
	
	switch ( msg->what )
	{
		case IM::MESSAGE:
		{
			int32 im_what=0;
			
			msg->FindInt32("im_what",&im_what);
			
			switch ( im_what )
			{
				case IM::REGISTER_CONTACTS:
				{
					if ( !fClient.icqclient.isConnected() )
						return B_ERROR;
					
					for ( int i=0; msg->FindString("id",i); i++ )
					{
						const char * id = msg->FindString("id",i);
						
						ICQ2000::ContactRef c = new ICQ2000::Contact( atoi(id) );
						fClient.icqclient.addContact( c );
					}
				}	break;
				
				case IM::UNREGISTER_CONTACTS:
				{
					if ( !fClient.icqclient.isConnected() )
						return B_ERROR;
					
					for ( int i=0; msg->FindString("id",i); i++ )
					{
						const char * id = msg->FindString("id",i);
						
						fClient.icqclient.removeContact( atoi(id) );
					}
				}	break;
				
				case IM::SET_STATUS:
				{
					const char * status = msg->FindString("status");
				
					if ( !status )
						return B_ERROR;
			
					if ( find_thread(ICQ_THREAD_NAME) == B_NAME_NOT_FOUND )
					{ // icq thread not running, start it
						LOG("icq", liLow, "Starting thread 'ICQ client'");
						
						fThread = spawn_thread(
							client_thread,
							ICQ_THREAD_NAME,
							B_NORMAL_PRIORITY,
							&fClient
						);
						
						resume_thread(fThread);
					} else
					{
						if ( strcmp(status,OFFLINE_TEXT) == 0 )
						{
							fClient.icqclient.setStatus(STATUS_OFFLINE);
						} else
						if ( strcmp(status,AWAY_TEXT) == 0 )
						{
							fClient.icqclient.setStatus(STATUS_AWAY);
							
							if ( msg->FindString("away_msg") != NULL )
							{ // let's use this away message eventually.
								fClient.setAwayMessage( msg->FindString("away_msg") );
							}
						} else
						if ( strcmp(status,ONLINE_TEXT) == 0 )
						{
							Progress( fMsgr, "ICQ: Going online..", 0.25 );
							fClient.icqclient.setStatus(STATUS_ONLINE);
						} else
						{ // invalid status code
							return B_ERROR;
						}
					}
				}	break;
		
				case IM::GET_CONTACT_INFO:
				{
					printf("Getting contact info\n");
					const char * id = msg->FindString("id");
					
					if ( !id )
						return B_ERROR;
					
					if ( !fClient.icqclient.isConnected() )
						return B_ERROR;
					
					fClient.icqclient.fetchSimpleContactInfo( new ICQ2000::Contact(atoi(id)) );
				}	break;
		
				case IM::SEND_MESSAGE:
				{
					const char * message_text = msg->FindString("message");
					const char * id = msg->FindString("id");
					const int32 charset = msg->FindInt32("charset");
					
					if ( !id )
						return B_ERROR;
					
					if ( !message_text )
						return B_ERROR;
					
					if ( !fClient.icqclient.isConnected() )
						return B_ERROR;
					
//					char newmsg[65536];
//					int32 state=0;
//					int32 dstlen = sizeof(newmsg);
//					int32 srclen = strlen(message_text);
					
					uint32 uin = atoi(id);
					
					NormalMessageEvent * ev = new NormalMessageEvent(
						new Contact(uin),
						message_text
					);
					
					fClient.icqclient.SendEvent( ev );
					
					// this is so fucked up.. but hey, we don't care :)
					// this is so we get the online status of the other end
					// we should keep a list of people we're already checking
					// and not add them again, but..
					ICQ2000::ContactRef c = new ICQ2000::Contact(uin);
					fClient.icqclient.addContact( c );
					
					// report message sent
					//msg->RemoveName("contact");
					//msg->ReplaceInt32("im_what", IM::MESSAGE_SENT);
					
					BMessage replyMsg(IM::MESSAGE);
					replyMsg.AddInt32("im_what", IM::MESSAGE_SENT);
					replyMsg.AddString("protocol", "icq");
					replyMsg.AddString("id", id);
					replyMsg.AddString("message", message_text);
					replyMsg.AddInt32("charset", charset);
					
					fMsgr.SendMessage(&replyMsg);
				}	break;
				case IM::SEND_AUTH_ACK:
				{
					bool authreply;
					
					if ( !fClient.icqclient.isConnected() )
						return B_ERROR;
					
					const char * id = msg->FindString("id");
					int32 button = msg->FindInt32("which");
					
					if (button == 0) {
						LOG("icq", liDebug, "Authorization granted to %s", id);
						authreply = true;												
					} else {
						LOG("icq", liDebug, "Authorization rejected to %s", id);
						authreply = false;					
					}
						
					ICQ2000::ContactRef c = new ICQ2000::Contact( atoi(id) );
					
					AuthAckEvent * ev = new AuthAckEvent(c, authreply);

					fClient.icqclient.SendEvent( ev );									
					
					if (authreply) {
						// Create a new contact now that we authorized him/her/it.
						BMessage im_msg(IM::MESSAGE);
						im_msg.AddInt32("im_what", IM::CONTACT_AUTHORIZED);
						im_msg.AddString("protocol", "icq");
						im_msg.AddString("id", id);
						im_msg.AddString("message", "" );
						//im_msg.AddInt32("charset",fEncoding);
	
						fMsgr.SendMessage(&im_msg);
					}
				}
				default:
					break;
			}
			
		}	break;
		default:
			break;
	}
	
	return B_OK;
}

const char *
ICQProtocol::GetSignature()
{
	return "icq";
}

const char *
ICQProtocol::GetFriendlySignature()
{
	return "ICQ";
}

BMessage
ICQProtocol::GetSettingsTemplate()
{
	BMessage main_msg(IM::SETTINGS_TEMPLATE);
	
	BMessage user_msg;
	user_msg.AddString("name","uin");
	user_msg.AddString("description", "UIN");
	user_msg.AddInt32("type",B_INT32_TYPE);
	
	BMessage pass_msg;
	pass_msg.AddString("name","password");
	pass_msg.AddString("description", "Password");
	pass_msg.AddInt32("type",B_STRING_TYPE);
	pass_msg.AddBool("is_secret", true);
	
	BMessage enc_msg;
	enc_msg.AddString("name","encoding");
	enc_msg.AddString("description","Text encoding");
	enc_msg.AddInt32("type", B_STRING_TYPE);
	enc_msg.AddString("valid_value", "ISO 8859-1");
	enc_msg.AddString("valid_value", "UTF-8");
	enc_msg.AddString("valid_value", "JIS");
	enc_msg.AddString("valid_value", "Shift-JIS");
	enc_msg.AddString("valid_value", "EUC");
	enc_msg.AddString("valid_value", "Windows 1251");
	enc_msg.AddString("default", "ISO 8859-1");
	
	main_msg.AddMessage("setting",&user_msg);
	main_msg.AddMessage("setting",&pass_msg);
	main_msg.AddMessage("setting",&enc_msg);
	
	return main_msg;
}

status_t
ICQProtocol::UpdateSettings( BMessage & msg )
{
	int32 uin=0;
	const char * password = NULL;
	const char * encoding = NULL;
	
	msg.FindInt32("uin",&uin);
	password = msg.FindString("password");
	encoding = msg.FindString("encoding");
	
	if ( uin == 0 || password == NULL || encoding == NULL )
		// invalid settings, fail
		return B_ERROR;
	
	if ( strcmp(encoding,"ISO 8859-1") == 0 )
	{
		fClient.setEncoding( B_ISO1_CONVERSION );
	} else
	if ( strcmp(encoding,"UTF-8") == 0 )
	{
		fClient.setEncoding( 0xffff );
	} else
	if ( strcmp(encoding,"JIS") == 0 )
	{
		fClient.setEncoding( B_JIS_CONVERSION );
	} else
	if ( strcmp(encoding,"Shift-JIS") == 0 )
	{
		fClient.setEncoding( B_SJIS_CONVERSION );
	} else
	if ( strcmp(encoding,"EUC") == 0 )
	{
		fClient.setEncoding( B_EUC_CONVERSION );
	} else
	if ( strcmp(encoding,"Windows 1251") == 0 )
	{
		fClient.setEncoding( B_MS_WINDOWS_1251_CONVERSION );
	} else
	{ // invalid encoding value
		return B_ERROR;
	}
	
	if ( fThread )
	{
		// we really should disconnect here if we're connected :/
		// ICQ won't like us changing UIN in the middle of everything.
	}
	
	fClient.icqclient.setUIN( uin );
	fClient.icqclient.setPassword( password );
	
	LOG("ICQ", liLow, "Settings updated");
	
	return B_OK;
}

uint32
ICQProtocol::GetEncoding()
{
	return fClient.fEncoding;
}

void
Progress( BMessenger & msgr, const char * message, float progress )
{
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::PROGRESS);
	msg.AddInt32("state", IM::impsConnecting);
	msg.AddString("progressID", "ICQ Login");
	msg.AddString("message", message);
	msg.AddString("protocol", "icq");
	msg.AddFloat("progress", progress);
	
	msgr.SendMessage( &msg );
}
