#ifndef OSCARREQCONN_H
#define OSCARREQCONN_H

#include "OSCARConnection.h"

class OSCARManager;
class SNAC;
class BufferReader;

class OSCARReqConn : public OSCARConnection {
	public:
							OSCARReqConn(const char *server, uint16 port,
								OSCARManager *man);
							~OSCARReqConn(void);
							
		virtual const char	*ConnName(void) const { return "OSCAR ReqConn"; };

				
	private:
		virtual status_t	HandleServiceControl(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleBuddyIcon(SNAC *snac, BufferReader *reader);
				
		OSCARManager		*fManager;
		BMessenger			fManMsgr;
			
		uint8				fState;
};

#endif
