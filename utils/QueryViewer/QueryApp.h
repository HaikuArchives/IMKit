#ifndef QUERYAPP_H
#define QUERYAPP_H

#include <Application.h>

#include "QueryWindow.h"

class QueryApplication : public BApplication {
	public:
							QueryApplication(void);
							~QueryApplication(void);
	
					bool	QuitRequested(void);
							
					void	ReadyToRun(void);
					void	MessageReceived(BMessage *msg);
	private:
				QueryWindow	*fWindow;
};

#endif
