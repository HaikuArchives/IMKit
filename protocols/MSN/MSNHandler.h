#ifndef MSNHANDLER_H
#define MSNHANDLER_H

#include "MSNConstants.h"

class MSNHandler {
	public:

		virtual inline		~MSNHandler() {};

//		To Protocol
		virtual status_t	ContactList(std::list<BString> *contacts) = 0;

//		From Protocol
		virtual status_t	StatusChanged(const char *nick, online_types type) = 0;
		virtual status_t	MessageFromUser(const char *nick, const char *msg) = 0;
		virtual status_t	UserIsTyping(const char *nick, typing_notification type) = 0;
		virtual status_t 	SSIBuddies(std::list<BString> buddies) = 0;
		virtual status_t	AuthRequest(list_types list, const char *passport,
								const char *displayname) = 0;
		virtual status_t	Error( const char * error_message )=0;
		virtual status_t	Progress( const char * id, const char * message, float progress )=0;
};

#endif
