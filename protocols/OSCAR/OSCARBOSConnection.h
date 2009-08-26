#ifndef OSCARBOSCONNECTION_H
#define OSCARBOSCONNECTION_H

#include "OSCARConstants.h"
#include "OSCARConnection.h"

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
typedef std::pair<char *, uint16> ServerAddress;

class OSCARManager;
class SNAC;
class BufferReader;

class OSCARBOSConnection : public OSCARConnection {
	public:
							OSCARBOSConnection(const char *server, uint16 port,
								OSCARManager *man,
								const char *name = "OSCAR BOS Connection");
							~OSCARBOSConnection();
						
		const char			*ConnName(void) const { return "OSCARConnection"; };

	private:
		virtual status_t	HandleServiceControl(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleAuthorisation(SNAC *snac, BufferReader *reader);

				
		BMessenger			fManMsgr;
		OSCARManager		*fManager;
};

#endif
