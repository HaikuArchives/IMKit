#ifndef BLOCKERAPP_H
#define BLOCKERAPP_H

#include <Application.h>
#include <Alert.h>
#include <TrackerAddon.h>

#include "../../common/IMKitUtilities.h"

class BlockerApp : public BApplication {
	public:
						BlockerApp(void);
						~BlockerApp(void);
				
				
		// Hooks
		virtual void 	MessageReceived(BMessage *msg);
		virtual void 	RefsReceived(BMessage *msg);
};

#endif
