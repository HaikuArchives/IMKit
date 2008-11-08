#ifndef PAPPLICATION_H
#define PAPPLICATION_H

#include "main.h"

#include <Application.h>
#include <Window.h>

class PWindow;
class BApplication;

class PApplication : public BApplication {
	public:
	
						PApplication(void);
						~PApplication(void);
						
		virtual bool	QuitRequested(void);
		virtual void	MessageReceived(BMessage *msg);

		virtual void	ReadyToRun(void);
				
	private:
		PWindow			*fWindow;
};

#endif

