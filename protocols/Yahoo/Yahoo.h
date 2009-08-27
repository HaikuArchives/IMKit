#ifndef IMKIT_YAHOO_H
#define IMKIT_YAHOO_H

#include <libim/Protocol.h>
#include <String.h>
#include <Messenger.h>

#include "YahooManager.h"

class YahooConnection;

class Yahoo : public IM::Protocol, public YahooManager
{
	public:
		Yahoo();
		virtual ~Yahoo();
		
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
	
		// YahooManager part begins here
		virtual void Error( const char * message, const char * who );

		virtual void GotMessage( const char * from, const char * msg );
		virtual void MessageSent( const char * to, const char * msg );
		
		virtual void LoggedIn();
		virtual void SetAway(bool);
		virtual void LoggedOut();
		
		virtual void TypeNotify(const char * who,int stat);
		
		virtual void GotBuddyList( std::list<std::string> & );
		virtual void GotContactsInfo( std::list<struct yahoo_buddy> & );
		virtual void GotBuddyIcon(const char *who, long size, const char* icon);
		virtual void BuddyStatusChanged( const char * who, const char * status );
		// YahooManager part ends here
		
	private:
		BMessenger	fServerMsgr;
		BString		fYahooID;
		BString		fPassword;
		
		YahooConnection *	fYahoo;
		
		void Progress( const char * id, const char * message, float progress );
};

#endif
