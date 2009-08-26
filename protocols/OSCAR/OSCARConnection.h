#ifndef OSCARCONNECTION_H
#define OSCARCONNECTION_H

#include "OSCARConstants.h"
#include "OSCARManager.h"

#include <Handler.h>
#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>

#include <list>
#include <vector>
#include <map>

#include <libim/Helpers.h>
#include <libim/Protocol.h>
#include <libim/Constants.h>

class Flap;
class TLV;
class SNAC;
class BufferReader;
class BLocker;

typedef std::pair<char *, uint16> ServerAddress;

// The Member Function Pointer for a Family handler / parser
typedef status_t (OSCARConnection::*FamilyHandler)(SNAC *, BufferReader *);
// Maps a family to a handler
typedef std::map<uint16, FamilyHandler> handler_t;

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
								OSCARManager *man, const char *name = "OSCAR Connection",
								conn_type type = connBOS);
							~OSCARConnection();
						
		void				MessageReceived(BMessage *msg);
				
		uint8				SupportedSNACs(void) const;
		uint16				SNACAt(uint8 index) const;
		bool				Supports(uint16 family);
		void				Support(uint16 family);
		
		status_t			Send(Flap *flap, send_time at = atBuffer);

		inline const char 	*Server(void) const { return fServer.String(); };
		inline uint16		Port(void) const { return fPort; };
		virtual const char	*ConnName(void) const { return "OSCARConnection"; };
		conn_type			ConnectionType(void) const { return fConnType; };
		
		uint8				State(void) const { return fState; };
		status_t			SetState(uint8 state);

		ServerAddress		ExtractServerDetails(char *details);
	private:
		status_t			LowLevelSend(Flap *flap);
		void				ClearQueue(void);		
		int32				ConnectTo(const char *hostname, uint16 port);
		static int32		Receiver(void *con);
		void				StartReceiver(void);
		void				StopReceiver(void);
		
		virtual status_t	HandleServiceControl(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleLocation(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleBuddyList(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleICBM(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleAdvertisement(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleInvitation(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleAdministrative(SNAC *snac, BufferReader *reader);
		virtual status_t	HandlePopupNotice(SNAC *snac, BufferReader *reader);
		virtual status_t	HandlePrivacy(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleUserLookup(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleUsageStats(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleTranslation(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleChatNavigation(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleChat(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleUserSearch(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleBuddyIcon(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleSSI(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleICQ(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleAuthorisation(SNAC *snac, BufferReader *reader);
		
		
		handler_t			fHandler;
		
		BString				fServer;
		uint16				fPort;
		
		flap_stack			fOutgoing;
		flap_stack			fOutgoingOnline;
		uint16				fOutgoingSeqNum;
		
		std::vector<uint16>		fSupportedSNACs;

		BMessenger			fManMsgr;
		BMessenger			*fSockMsgr;
		BMessageRunner		*fRunner;
		BMessageRunner		*fKeepAliveRunner;
		
		int32				fSock;
		
		uint8				fState;
		thread_id			fThread;
		
		uint32				fRequestID;
		OSCARManager		*fManager;
		conn_type			fConnType;
		
#ifndef BONE_BUILD
		BLocker				*fSocketLock;
#endif
};

#endif
