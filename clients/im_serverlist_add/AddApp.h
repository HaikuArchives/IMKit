#ifndef SERVERLISTADD_H
#define SERVERLISTADD_H

#include <Application.h>
#include <Alert.h>
#include <TrackerAddon.h>

#include "../../common/IMKitUtilities.h"

class ServerListAddApp : public BApplication {
	public:
						ServerListAddApp(void);
						~ServerListAddApp(void);
				
				
		// Hooks
		virtual void 	MessageReceived(BMessage *msg);
		virtual void 	RefsReceived(BMessage *msg);
};

#endif
