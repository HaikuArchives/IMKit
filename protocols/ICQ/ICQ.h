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
 * See the sourcecode for shell.cpp for more info.
 *
 */

/* Modified to work as a protcol add-on for the BeOS Instant Messaging Kit
 * by Mikael Eiman <mikael@eiman.tv>
 */

#include <iostream>
#include <map>

#include <libicq2000/Client.h>
#include <libicq2000/events.h>
#include <libicq2000/constants.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include "Select.h"

#include <libim/Protocol.h>
#include <Messenger.h>
#include <OS.h>

using namespace ICQ2000;
using namespace std;
using namespace IM;

// ------------------------------------------------------------------
//  global declarations (ughh..)
// ------------------------------------------------------------------
void usage(const char *c);
void processCommandLine(int argc, char *argv[]);

char *password, *shellcmd;
unsigned int uin;
bool respond;

void Progress( BMessenger &, const char * message, float );

// ------------------------------------------------------------------
//  Simple Client declaration
// ------------------------------------------------------------------

class SimpleClient : public SigC::Object {
 private:
  friend class ICQProtocol;
  
  ICQ2000::Client icqclient;
  Select input;
  std::map<int, SigC::Connection> m_sockets;

  BMessenger	fMsgr;
  int32			fEncoding;
  
  string		fAwayMessage;
  
 protected:

  // -- Callbacks from libicq2000 --
  void connected_cb(ConnectedEvent *c);
  void disconnected_cb(DisconnectedEvent *c);
  void message_cb(MessageEvent *c);
  void logger_cb(LogEvent *c);
  void contact_status_change_cb(StatusChangeEvent *ev);
  void socket_cb(SocketEvent *ev);
  void contact_userinfo_change_cb( UserInfoChangeEvent *ev );
  void server_based_contact_list_cb( ServerBasedContactEvent * ev );
  void self_status_change_cb( StatusChangeEvent * ev );
  //void want_auto_resp_cb( ICQMessageEvent* ); // Disabled to see if it stops the ICQ crashes.
  
  // -- Callbacks from our Select object --
  void select_socket_cb(int fd, Select::SocketInputCondition cond);
  
 public:
  SimpleClient(unsigned int uin, const string& pass);
  SimpleClient();

  void run();
  
  void setMessenger( BMessenger );
  void setEncoding( int32 );
  void setAwayMessage( string );
};

class ICQProtocol : public IM::Protocol
{
	public:
		ICQProtocol();
		virtual ~ICQProtocol();
		
		virtual status_t Init( BMessenger );
		virtual status_t Shutdown();
		
		virtual status_t Process( BMessage * );
		
		virtual const char * GetSignature();
		virtual const char * GetFriendlySignature();
	
		virtual BMessage GetSettingsTemplate();
		
		virtual status_t UpdateSettings( BMessage & );
		
		virtual uint32 GetEncoding();
		
	private:
		#define ICQ_THREAD_NAME "ICQ client"
		
		BMessenger		fMsgr;
		SimpleClient	fClient;
		thread_id		fThread;
};
