#ifndef MSNCONSTANTS_H
#define MSNCONSTANTS_H

extern const char *kProtocolName;
extern const char *kThreadName;

extern const char *kClientVer;
extern const char *kProtocolsVers;

extern const int16 kDefaultPort;

extern const uint32 kOurCaps;
extern const char *kClientID;
extern const char *kClientCode;

enum online_types {
	otOnline,
	otAway,
	otBusy,
	otIdle,
	otBRB,
	otPhone,
	otLunch,
	otInvisible,
	otBlocked,
	otConnecting,
	otOffline
};

enum list_types {
	ltNone = 0,
	ltForwardList = 1,
	ltAllowList = 2,
	ltBlockList	= 4,
	ltReverseList = 8,
};

enum typing_notification {
	tnStartedTyping,
	tnStillTyping,
	tnStoppedTyping
};

enum client_caps {
	ccMobileDev = 0x00000001,
	ccMSNExplorer8 = 0x00000002,
	ccReceiveInk = 0x00000004,
	ccSendInk = 0x00000008,
	ccVideoChat = 0x00000010,
	ccMultiPacketMessaging = 0x00000020,
	ccHasMobileDev = 0x00000040,
	ccHasDirectDev = 0x00000080,
	ccWebMSNMessenger = 0x00000200,
	ccOfficialLiveClient = 0x00000800,
	ccHasMSNSpace = 0x00001000,
	ccMediaCenterClient = 0x00002000,
	ccSupportsDirectIM = 0x00004000,
	ccSupportsWinks = 0x00008000,
	ccSupportsMSNSearch = 0x00010000,
	ccSupportsVoiceClips = 0x00040000,
	ccSupportsSecureChannels = 0x00080000,
	ccSupportsSIPInvitations = 0x00100000,
	ccSupportsSharingFolders = 0x00400000,
	ccMSNC1 = 0x10000000,	// MSN Messenger 6.0
	ccMSNC2 = 0x20000000,	// MSN Messenger 6.1
	ccMSNC3 = 0x30000000,	// MSN Messenger 6.2
	ccMSNC4 = 0x40000000,	// MSN Messenger 7.0
	ccMSNC5 = 0x50000000,	// MSN Messenger 7.5
	ccMSNC6 = 0x60000000,	// MSN Messenger 8.0
	ccMSNC7 = 0x70000000,	// MSN Messenger 8.1
	ccMSNC8 = 0x80000000,	// MSN Messenger 8.5
};

#endif
