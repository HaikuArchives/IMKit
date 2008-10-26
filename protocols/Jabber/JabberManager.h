#ifndef Jabber_MANAGER_H
#define Jabber_MANAGER_H

#include <list>
#include <string>

/**
	
*/

class JabberManager
{
	public:
		// who can be NULL if it's a general message
		virtual void Error( const char * message, const char * who )=0;
		
		virtual void GotMessage( const char * from, const char * msg )=0;
		virtual void MessageSent( const char * to, const char * msg )=0;
		
		virtual void LoggedIn()=0;
		virtual void SetAway(bool)=0;
		virtual void LoggedOut()=0;
		
		//virtual void GotBuddyList( list<string> & )=0;
		virtual void BuddyStatusChanged( const char * who, const char * status )=0;
};

#endif
