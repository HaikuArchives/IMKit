#include <iostream>
#include <map>
#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <libim/Protocol.h>
#include <libim/Constants.h>
#include <Messenger.h>
#include <OS.h>
#include <File.h>

#include "OSCARHandler.h"

using namespace IM;

class OSCARManager;

class AIMProtocol : public IM::Protocol, public OSCARHandler {
	public:
							AIMProtocol(void);
		virtual				~AIMProtocol(void);

		// IM::Protocol hooks
		status_t		 	Init(BMessenger);
		status_t 			Shutdown(void);	
		status_t 			Process(BMessage *msg);
		const char 			*GetSignature(void);
		const char 			*GetFriendlySignature(void);
		status_t 			UpdateSettings(BMessage &settings);
		uint32 				GetEncoding(void);

		// OSCARHandler hooks		
		status_t			Error(const char *msg);
		status_t			Progress(const char * id, const char * message,
								float progress);
		status_t			StatusChanged(const char *nick, online_types status,
								bool mobileUser = false);
		status_t			MessageFromUser(const char *nick, const char *msg,
								bool autoReply = false);
		status_t			UserIsTyping(const char *nick, typing_notification type);
		status_t		 	SSIBuddies(std::list<BString> buddies);
		status_t			BuddyIconFromUser(const char *nick, const uchar *icon,
								uint32 length);
		status_t			AuthRequestFromUser(char *nick, char *reason);

		char				*RoastPassword(const char *pass);
		void				FormatMessageText(BString &message);

	private:
		BString 			NormalizeNick(const char *nick);
		BString 			GetScreenNick(const char *nick);
	
		BMessenger			fMsgr;
		thread_id			fThread;
		OSCARManager		*fManager;
		char				*fScreenName;
		char				*fPassword;
		uint32				fEncoding;
		
		std::map<std::string,BString>	fNickMap;
};

