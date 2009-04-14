#ifndef CHATAPP_H
#define CHATAPP_H

#include "main.h"

class BApplication;
class ChatWindow;
class RunView;

typedef map<entry_ref, RunView *> RunMap;
extern const char *kDefaultPeopleHandler;

class ChatApp : public BApplication
{
	public:
						ChatApp();
			virtual		~ChatApp();
		
		virtual bool	QuitRequested();
		
		virtual void 	MessageReceived( BMessage * );
		virtual void 	RefsReceived( BMessage * );
		
				bool	IsQuiting();
		
				void	Flash( BMessenger );
				void	NoFlash( BMessenger );
			
			status_t	StoreRunView(/*const char *id*/ entry_ref, RunView *rv);
			RunView		*GetRunView(/*const char *id*/ entry_ref);
		
		BList			chat_windows;
		
	private:
		ChatWindow		*findWindow( entry_ref & );
		BString			fPeopleHandler;
		
		IM::Manager		*fMan;
		bool			fIsQuiting;
		RunMap			fRunViews;
};

#endif
