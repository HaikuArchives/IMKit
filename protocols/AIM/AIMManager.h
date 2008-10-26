#ifndef AIMMANAGER_H
#define AIMMANAGER_H

#include <Handler.h>
#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>
#include <String.h>

#ifdef BONE_BUILD
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#else
	#include <net/socket.h>
	#include <net/netdb.h>
#endif

#include <libim/Helpers.h>

#include <list>
#include <map>

#include "FLAP.h"
#include "TLV.h"
#include "Buddy.h"

class OSCARConnection;
class AIMReqConn;
class OSCARHandler;

typedef map<BString, Buddy *> buddymap;
typedef map<uint16, OSCARConnection *> pfc_map; // Pending family / connection map
typedef list<Flap *> flap_stack;
typedef list<OSCARConnection *> connlist;

enum {
	AMAN_PULSE = 'ampu',
	AMAN_KEEP_ALIVE = 'amka',
	AMAN_GET_SOCKET = 'amgs',
	AMAN_FLAP_OPEN_CON = 'amoc',
	AMAN_FLAP_SNAC_DATA = 'amsd',
	AMAN_FLAP_ERROR = 'amfe',
	AMAN_FLAP_CLOSE_CON = 'amcc',
	AMAN_NEW_CONNECTION = 'amnc',
	AMAN_CLOSED_CONNECTION = 'amcd',

	AMAN_STATUS_CHANGED = 'amsc',
	AMAN_NEW_CAPABILITIES = 'amna'
};

//const char kEncoding[] = "text/aolrtf; charset=\"us-ascii\"";
const char kEncoding[] = "text/aolrtf; charset=\"utf-8\"";
const char kBuddyIconCap [] = {
	0x09, 0x46, 0x13, 0x46,
	0x4c, 0x7f, 0x11, 0xd1,
	0x82, 0x22, 0x44, 0x45,
	0x53, 0x54, 0x00, 0x00
};
const uint16 kBuddyIconCapLen = 16;

class AIMManager : public BLooper {
	public:
							AIMManager(OSCARHandler *handler);
							~AIMManager(void);
						
		status_t			Send(Flap *f);
			
		void				MessageReceived(BMessage *message);
			
		status_t			MessageUser(const char *screenname, const char *message);
		status_t			AddBuddy(const char *buddy);
		status_t			AddBuddies(list<char *>buddies);
		int32				Buddies(void) const;
		status_t			RemoveBuddy(const char *buddy);
		status_t			RemoveBuddies(list<char *>buddies);

		status_t			Login(const char *server, uint16 port,
								const char *username, const char *password);
		uchar				IsConnected(void) const;
		status_t			LogOff(void);
			
		status_t			SetProfile(const char *profile);
		status_t			SetAway(const char *message);
		status_t			TypingNotification(const char *buddy, uint16 typing);
		status_t			SetIcon(const char *icon, int16 size);

		inline uchar		ConnectionState(void) const {
								return fConnectionState;
							};
		const char 			*Profile(void) const {
								return fProfile.String();
							};
		
		status_t			Progress(const char *id, const char *msg,
								float progress);
		status_t			Error(const char *msg);

	private:
		status_t			ClearConnections(void);
		status_t			ClearWaitingSupport(void);
		char				*EncodePassword(const char *pass);
		char				*ParseMessage(TLV *tlv);
		
		virtual status_t	HandleServiceControl(BMessage *msg);
		virtual status_t	HandleICBM(BMessage *msg);
		virtual status_t	HandleLocation(BMessage *msg);
		virtual status_t	HandleBuddyList(BMessage *msg);
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
			
		buddymap			fBuddy;
		connlist			fConnections;
		flap_stack			fWaitingSupport;
		pfc_map				fPendingConnections;

		uint16				fSSILimits[kSSILimitCount];
		
		BMessageRunner		*fRunner;
		BMessageRunner		*fKeepAliveRunner;
		uchar				fConnectionState;

		char				*fOurNick;
		BString				fProfile;
		BString				fAwayMsg;
		OSCARHandler		*fHandler;
		char				*fIcon;
		int16				fIconSize;
		int16				fSSIItems;
};

#endif
