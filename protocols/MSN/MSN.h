#ifndef MSN_H
#define MSN_H

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

#include "MSNManager.h"
#include "MSNHandler.h"

using namespace IM;

class MSNHandler;

class MSNProtocol : public IM::Protocol, public MSNHandler {
	public:
								MSNProtocol(void);
		virtual 				~MSNProtocol(void);
		
		virtual status_t		Init(BMessenger msgr);
		virtual status_t		Shutdown(void);
		
		virtual status_t		Process(BMessage *msg);
		
		virtual const char 		*GetSignature(void);
		virtual const char 		*GetFriendlySignature(void);
		
		virtual status_t 		UpdateSettings(BMessage &settings);
		
		virtual uint32 			GetEncoding(void);
		
//		To Protocol
		status_t				ContactList(list<BString> *contacts);
		
//		From Protocol		
		status_t				StatusChanged(const char *nick, online_types status);
		status_t				MessageFromUser(const char *passport, const char *msg);
		status_t				UserIsTyping(const char *nick, typing_notification type);
		status_t			 	SSIBuddies(list<BString> buddies);
		status_t				AuthRequest(list_types list, const char *passport, const char *displayname);
		status_t				Error(const char * error_message);
		status_t				Progress(const char * id, const char * message, float progress);

	private:
		BString					NormalizeNick(const char *nick);
		BString					GetScreenNick(const char *nick);
	
		BMessenger				fMsgr;
		thread_id				fThread;
		MSNManager				*fManager;
		BString					fPassport;
		BString					fPassword;
		BString					fDisplayName;
		
		map<string,BString>		fNickMap;
};

#endif
