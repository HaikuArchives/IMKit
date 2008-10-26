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

#include "AIMManager.h"
#include "OSCARHandler.h"

using namespace IM;

class AIMHandler;

class AIMProtocol : public IM::Protocol, public OSCARHandler
{
	public:
				AIMProtocol();
		virtual ~AIMProtocol();
		
		virtual status_t Init( BMessenger );
		virtual status_t Shutdown();
		
		virtual status_t Process( BMessage * );
		
		virtual const char * GetSignature();
		virtual const char * GetFriendlySignature();
	
		virtual BMessage GetSettingsTemplate();
		
		virtual status_t UpdateSettings( BMessage & );
		
		virtual uint32 GetEncoding();
		
			status_t	Error(const char *msg);
			status_t	Progress(const char * id, const char * message,
							float progress);
			
			
			status_t	StatusChanged(const char *nick, online_types status);
			status_t	MessageFromUser(const char *nick, const char *msg,
							bool autoReply = false);
			status_t	UserIsTyping(const char *nick, typing_notification type);
			status_t 	SSIBuddies(list<BString> buddies);
			status_t	BuddyIconFromUser(const char *nick, const uchar *icon,
							uint32 length);

	private:
			BString 	NormalizeNick(const char *nick);
			BString 	GetScreenNick(const char *nick);
	
		BMessenger		fMsgr;
		thread_id		fThread;
		AIMManager		*fManager;
		char			*fScreenName;
		char			*fPassword;
		uint32			fEncoding;
		
		map<string,BString>		fNickMap;
};

