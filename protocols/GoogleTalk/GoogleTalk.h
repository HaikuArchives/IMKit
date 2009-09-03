/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef IMKIT_GoogleTalk_H
#define IMKIT_GoogleTalk_H

#include <list>

#include <String.h>
#include <Messenger.h>

#include <libim/Protocol.h>

#include <libjabber/JabberHandler.h>

#include "JabberManager.h"

class GoogleTalkConnection;
class JabberSSLPlug;

class GoogleTalk : public IM::Protocol, public JabberManager, public JabberHandler {
public:
		GoogleTalk();
		~GoogleTalk();

		// IM::Protocol part begins here
		// messenger to im_server
		virtual status_t Init( BMessenger );

		// called before unloading from memory
		virtual status_t Shutdown();

		// process message
		virtual status_t Process( BMessage * );

		// Get name of protocol
		virtual const char * GetSignature();
		virtual const char * GetFriendlySignature();

		// settings changed
		virtual status_t UpdateSettings( BMessage & );

		// preferred encoding of messages
		virtual uint32 GetEncoding();
		// IM::Protocol part ends here

		// JabberManager part begins here
		virtual void Error( const char * message, const char * who );

		virtual void GotMessage( const char * from, const char * msg );
		virtual void MessageSent( const char * to, const char * msg );

		virtual void LoggedIn();
		virtual void SetAway(bool);
		virtual void LoggedOut();

		//virtual void GotBuddyList( std::list<string> & );
		virtual void BuddyStatusChanged( const char * who, const char * status );
		virtual void BuddyStatusChanged( JabberContact* who );
		virtual void BuddyStatusChanged( JabberPresence* who );
		// JabberManager part ends here

private:
		JabberSSLPlug*	fPlug;
		BMessenger	fServerMsgr;

		BString		fUsername;
		BString		fServer;
		BString		fPassword;

		typedef std::list<BString>	StrList; // new buddy added when off-line.
		StrList*			fLaterBuddyList;

		//special client
		//StrList				fSpecialUID;
		BMessage				fSpecialUID;

		//temp?
		bool 	fAuth;
		bool 	fRostered;
		bool 	fAgent;
		float	fPerc;
		bool	fFullLogged;

		void Progress( const char * id, const char * message, float progress );

		JabberContact*	getContact(const char* id);
		void			SendContactInfo(const char* id);
		void			AddStatusString(JabberPresence* who ,BMessage* to);	

		void			CheckLoginStatus();

// Callbacks from JabberHandler
protected:
	virtual	void				Authorized();
	virtual void 				Message(JabberMessage * message); 
	virtual void 				Presence(JabberPresence * presence);
	virtual void 				Roster(RosterList * roster);
	virtual void 				Agents(AgentList * agents);
	virtual void 				Disconnected(const BString & reason) ;
	virtual void 				SubscriptionRequest(JabberPresence * presence) ;
	virtual void 				Registration(JabberRegistration * registration) ;	
	virtual	void				Unsubscribe(JabberPresence * presence);
};

#endif	// IMKIT_GoogleTalk_H
