#ifndef OSCARMANAGER_H
#define OSCARMANAGER_H

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

#include "OSCARConstants.h"

#include <list>
#include <vector>
#include <map>

class OSCARManager;
class OSCARConnection;
class AIMReqConn;
class OSCARHandler;
class Group;
class Buddy;
class TLV;
class Flap;
class SNAC;
class BufferReader;

typedef map<BString, Buddy *> buddymap;
typedef map<uint16, OSCARConnection *> pfc_map; // Pending family / connection map
typedef list<Flap *> flap_stack;
typedef list<OSCARConnection *> connlist;
typedef map<uint16, Group *> group_t;
typedef map<uint16, bool> identity_t;
typedef vector<BString> grouplist_t;

// The Member Function Pointer for a Family handler / parser
typedef status_t (OSCARManager::*FamilyManHandler)(SNAC *, BufferReader *);
// Maps a family to a handler
typedef map<uint16, FamilyManHandler> manhandler_t;

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

class OSCARManager : public BLooper {
	public:
							OSCARManager(OSCARHandler *handler, const char *protocol);
							~OSCARManager(void);
						
		const char			*Protocol(void);
						
		status_t			Send(Flap *f);
			
		void				MessageReceived(BMessage *message);
			
		status_t			MessageUser(const char *screenname, const char *message);
		status_t			AddSSIBuddy(const char *buddy, grouplist_t groups);
		status_t			AddBuddy(const char *buddy);
		status_t			AddBuddies(list<char *>buddies);
		int32				Buddies(void) const;
		status_t			RemoveBuddy(const char *buddy);
		status_t			RemoveBuddies(list<char *>buddies);
		Buddy				*GetBuddy(const char *screenname);

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
		uint16				GetNewItemId(void);
		
		virtual status_t	HandleServiceControl(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleICBM(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleLocation(SNAC *snac, BufferReader *reader);
		virtual status_t	HandleBuddyList(SNAC *snac, BufferReader *reader);
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
		
		manhandler_t		fSNACHandler;
		
		BString				fProtocol;
		
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
		group_t				fGroups;
		identity_t			fItemIds;
};

#endif
