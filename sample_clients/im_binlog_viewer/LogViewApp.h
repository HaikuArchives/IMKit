#ifndef LOGVIEWAPP_H
#define LOGVIEWAPP_H

#include <Application.h>
#include <Alert.h>
#include <TrackerAddOn.h>

#include "../../common/IMKitUtilities.h"
#include "LogWin.h"

class LogViewApp : public BApplication {
	public:
						LogViewApp(void);
						~LogViewApp(void);
				
				
		virtual void 	MessageReceived(BMessage *msg);
		virtual void 	RefsReceived(BMessage *msg);
};

#endif
