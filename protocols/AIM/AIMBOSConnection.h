#ifndef AIMCONNECTION_H
#define AIMCONNECTION_H

#include "AIMManager.h"

#include "OSCARConstants.h"
#include "OSCARConnection.h"
#include "FLAP.h"
#include "TLV.h"
#include "SNAC.h"

#include <Handler.h>
#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>

#include <list>
#include <vector>

#include <libim/Helpers.h>
#include <libim/Protocol.h>
#include <libim/Constants.h>

extern const int32 kUnhandled;
typedef pair <char *, uint16> ServerAddress;

class AIMManager;
class OSCARConnection;

class AIMBOSConnection : public OSCARConnection {
	public:
						AIMBOSConnection(const char *server, uint16 port,
							AIMManager *man, const char *name = "AIMBOS Connection");
						~AIMBOSConnection();
						
		const char		*ConnName(void) const { return "AIMConnection"; };

	private:
		virtual status_t	HandleServiceControl(BMessage *msg);
		virtual status_t	HandleAuthorisation(BMessage *msg);

				
		BMessenger		fManMsgr;
		AIMManager		*fManager;
};

#endif
