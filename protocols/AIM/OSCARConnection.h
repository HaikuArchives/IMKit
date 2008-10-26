#ifndef OSCARCONNECTION_H
#define OSCARCONNECTION_H

#include "AIMManager.h"

#include "OSCARConstants.h"
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

typedef pair <char *, uint16> ServerAddress;
class AIMManager;
const status_t kUnhandled = -1;

enum send_time {
	atImmediate,
	atBuffer,
	atOnline
};

enum conn_type {
	connBOS,
	connReq,
};

class OSCARConnection : public BLooper {
	public:
						OSCARConnection(const char *server, uint16 port,
							AIMManager *man, const char *name = "OSCAR Connection",
							conn_type type = connBOS);
						~OSCARConnection();
						
		void			MessageReceived(BMessage *msg);
				
		uint8			SupportedSNACs(void) const;
		uint16			SNACAt(uint8 index) const;
		bool			Supports(const uint16 family) const;
		void			Support(uint16 family);
		
		status_t		Send(Flap *flap, send_time at = atBuffer);

		inline const char
						*Server(void) const { return fServer.String(); };
		inline uint16	Port(void) const { return fPort; };
		virtual 
			const char	*ConnName(void) const { return "OSCARConnection"; };
		conn_type		ConnectionType(void) const { return fConnType; };
		
		uint8			State(void) const { return fState; };
		status_t		SetState(uint8 state);

		ServerAddress	ExtractServerDetails(char *details);
	private:
		status_t		LowLevelSend(Flap *flap);
		void			ClearQueue(void);		
		int32			ConnectTo(const char *hostname, uint16 port);
		static int32	Receiver(void *con);
		void			StartReceiver(void);
		void			StopReceiver(void);
		
		virtual status_t	HandleServiceControl(BMessage *msg);
		virtual status_t	HandleLocation(BMessage *msg);
		virtual status_t	HandleBuddyList(BMessage *msg);
		virtual status_t	HandleICBM(BMessage *msg);
		virtual status_t	HandleAdvertisement(BMessage *msg);
		virtual status_t	HandleInvitation(BMessage *msg);
		virtual status_t	HandleAdministrative(BMessage *msg);
		virtual status_t	HandlePopupNotice(BMessage *msg);
		virtual status_t	HandlePrivacy(BMessage *msg);
		virtual status_t	HandleUserLookup(BMessage *msg);
		virtual status_t	HandleUsageStats(BMessage *msg);
		virtual status_t	HandleTranslation(BMessage *msg);
		virtual status_t	HandleChatNavigation(BMessage *msg);
		virtual status_t	HandleChat(BMessage *msg);
		virtual status_t	HandleUserSearch(BMessage *msg);
		virtual status_t	HandleBuddyIcon(BMessage *msg);
		virtual status_t	HandleSSI(BMessage *msg);
		virtual status_t	HandleICQ(BMessage *msg);
		virtual status_t	HandleAuthorisation(BMessage *msg);
				
		BString			fServer;
		uint16			fPort;
		
		flap_stack		fOutgoing;
		flap_stack		fOutgoingOnline;
		uint16			fOutgoingSeqNum;
		
		vector<uint16>	fSupportedSNACs;

		BMessenger		fManMsgr;
		BMessenger		*fSockMsgr;
		BMessageRunner	*fRunner;
		BMessageRunner	*fKeepAliveRunner;
		
		int16			fSock;
		
		uint8			fState;
		thread_id		fThread;
		
		uint32			fRequestID;
		
		AIMManager		*fManager;
		
		conn_type		fConnType;
};

#endif
