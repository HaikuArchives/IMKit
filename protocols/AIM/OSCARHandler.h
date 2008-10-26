#ifndef OSCARHANDLER_H
#define OSCARHANDLER_H

#include "OSCARConstants.h"

class OSCARHandler {
	public:

		virtual inline		~OSCARHandler() {};

		virtual status_t	Error(const char *msg) = 0;
		virtual status_t	Progress(const char * id, const char * message,
								float progress ) = 0;

		virtual status_t	StatusChanged(const char *nick, online_types type) = 0;
		virtual status_t	MessageFromUser(const char *nick, const char *msg, bool isAutoReply = false) = 0;
		virtual status_t	UserIsTyping(const char *nick, typing_notification type) = 0;
		virtual status_t 	SSIBuddies(list<BString> buddies) = 0;
		virtual status_t	BuddyIconFromUser(const char *nick, const uchar *data,
								uint32 length) = 0;
};

#endif
