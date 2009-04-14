#include "DeskbarIcon.h"

#include "AccountInfo.h"
#include "common/BubbleHelper.h"
#include "common/GenericStore.h"
#include "common/IMKitUtilities.h"

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>

#include <Message.h>
#include <stdio.h>
#include <Roster.h>
#include <MenuItem.h>
#include <Application.h>
#include <Window.h>
#include <Deskbar.h>

#include <storage/FindDirectory.h>
#include <storage/File.h>
#include <storage/Path.h>
#include <storage/Resources.h>

#include "preflet/PResources.h"

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

//#pragma mark Constants

const char *kLogName = "Deskbar";

const char *kTrackerQueryVolume = "_trk/qryvol1";
const char *kTrackerQueryPredicate = "_trk/qrystr";
const int8 kStatusOnline = 0;
const int8 kStatusAway = 1;
const int8 kStatusOffline = 2;

// Status setting messages
const int32 kMsgSetStatus = 'set1';
const int32 kMsgSetStatusOnline = 'set2';
const int32 kMsgSetStatusAway = 'set3';
const int32 kMsgSetStatusOffline = 'set4';

// Settings messages
const int32 kMsgOpenSettings = 'opse';
const int32 kMsgReloadSettings = 'upse';

// Server run/quit messages
const int32 kMsgCloseIMServer = 'imqu';
const int32 kMsgStartIMServer = 'imst';

// Query messages
const int32 kMsgOpenQuery = 'opqu';
const int32 kMsgLaunchFile = 'lafi';
const int32 kMsgOpenQueryDir = 'opqd';
const int32 kMsgQueryUpdated = 'qlup';

// Misc messages
const int32 kMsgSettingsWindowClose = 'swcl';

class AccountStore : public IM::GenericMapStore<BString, AccountInfo *> {
};

//#pragma mark Extern C

extern "C" {
	BView *instantiate_deskbar_item() {
		LOG(kLogName, liHigh, "IM: Instantiating Deskbar item");
		return new IM_DeskbarIcon();
	}
}

//#pragma mark -

BArchivable *IM_DeskbarIcon::Instantiate(BMessage * archive) {
	if (!validate_instantiation(archive,"IM_DeskbarIcon")) {
		LOG(kLogName, liHigh, "IM_DeskbarIcon::Instantiate(): Invalid archive");
		return NULL;
	};
	
	LOG(kLogName, liHigh, "IM_DeskbarIcon::Instantiate() ok");
	
	return new IM_DeskbarIcon(archive);
};


IM_DeskbarIcon::IM_DeskbarIcon()
	: BView(BRect(0,0,15,15), DESKBAR_ICON_NAME, B_FOLLOW_NONE, B_WILL_DRAW) {
	LOG(kLogName, liHigh, "IM_DeskbarIcon::IM_DeskbarIcon()");

	_init();
};

IM_DeskbarIcon::IM_DeskbarIcon(BMessage * archive)
	: BView(archive) {
	LOG(kLogName, liHigh, "IM_DeskbarIcon::IM_DeskbarIcon(BMessage*)");

	_init();
};

IM_DeskbarIcon::~IM_DeskbarIcon() {
	LOG(kLogName, liHigh, "IM_DeskbarIcon::~IM_DeskbarIcon()");

	delete fTip;
	delete fAwayIcon;
	delete fOnlineIcon;
	delete fOfflineIcon;
	delete fGenericIcon;

	if (fMsgRunner) {
		delete fMsgRunner;
	};
};

void IM_DeskbarIcon::_init() {
	fAccount = new AccountStore();

	LOG(kLogName, liHigh, "IM_DeskbarIcon::_init()");

	image_info info;
	if (our_image(info) != B_OK) {
		return;
	};

	BFile file(info.name, B_READ_ONLY);
	
	if (file.InitCheck() < B_OK) {
		return;
	};

	BResources resources(&file);
#ifdef __HAIKU__
	if (resources.InitCheck() < B_OK) {
		return;
	};
#endif

	fAwayIcon = GetIconFromResources(&resources, kDeskbarAwayIcon, B_MINI_ICON);
	if (fAwayIcon == NULL) {
		LOG(kLogName, liHigh, "Error loading fAwayIcon");
	};

	fOnlineIcon = GetIconFromResources(&resources, kDeskbarOnlineIcon, B_MINI_ICON);
	if (fOnlineIcon == NULL) {
		LOG(kLogName, liHigh, "Error loading fOnlineIcon");
	};

	fOfflineIcon = GetIconFromResources(&resources, kDeskbarOfflineIcon, B_MINI_ICON);
	if (fOfflineIcon == NULL) {
		LOG(kLogName, liHigh, "Error loading fOfflineIcon");
	};

	fGenericIcon = GetIconFromResources(&resources, kDeskbarGenericIcon, B_MINI_ICON);
	if (fGenericIcon == NULL) {
		LOG(kLogName, liHigh, "Error loading fGenericIcon");
	};

	// Initial icon is the Offline icon
	fCurrIcon = fModeIcon = fOfflineIcon;
	fStatus = kStatusOffline;

	fFlashCount = 0;
	fBlink = 0;
	fMsgRunner = NULL;
	
	fShouldBlink = true;
	
	fTip = new BubbleHelper();

	fDirtyStatusMenu = true;
	fStatusMenu = NULL;
	
	fDirtyStatus = true;
	fMenu = NULL;
	
	SetDrawingMode(B_OP_OVER);
	
	getProtocolStates();
	
	fQueryMenu = NULL;

	BPath userDir;
	find_directory(B_USER_SETTINGS_DIRECTORY, &userDir, true);

	BPath middlePath = userDir;
	middlePath.Append("im_kit/_MIDDLE_CLICK_ACTION_");
	
	BEntry middleAction(middlePath.Path(), true);
	if ((middleAction.InitCheck() != B_OK) || (middleAction.Exists() == false)) {
		middlePath.SetTo("/boot/home/people");
		get_ref_for_path(middlePath.Path(), &fMiddleClickRef);
	} else {
		middleAction.GetRef(&fMiddleClickRef);
	};

#ifdef ZETA
	/*
	 * This is straightforward and unsafe as the location of the locale
	 * files may change. We could put the files relative to the im_server
	 * install location, but this is quite ugly.
	 */
	BPath path( "/boot/apps/Internet/IMKit/Language/Dictionaries/im_server" );
	if (path.InitCheck() == B_OK) {
		be_locale.LoadLanguageFile(path.Path());
	};
#endif
};

void IM_DeskbarIcon::getProtocolStates(void) {
	BMessage protStatus;
	IM::Manager man;
	
	if (man.SendMessage(new BMessage(IM::GET_OWN_STATUSES), &protStatus) != B_OK) {
		LOG(kLogName, liHigh, "Error getting statuses");
	};

	fTipText = "Online Status:";
	
	fAccount->Clear();
	
	BString instanceID;
	
	LOG(kLogName, liDebug, "IM_DeskbarIcon::getProtocolStates: ", &protStatus);
	
	for (int32 i = 0; protStatus.FindString("instance_id", i, &instanceID) == B_OK; i++) {
		const char *protocol = NULL;
		const char *userfriendly = NULL;
		const char *status = NULL;
		const char *account = NULL;
		
		if (protStatus.FindString("protocol",i, &protocol) != B_OK) protocol = _T("Unknown protocol");
		if (protStatus.FindString("userfriendly",i, &userfriendly) != B_OK) userfriendly = _T("Unknown");
		if (protStatus.FindString("status", i, &status) != B_OK) status = _T("Unknown status");
		if (protStatus.FindString("account_name", i, &account) != B_OK) account = _T("Unknown account");

		AccountInfo *info = new AccountInfo(instanceID.String(), account, protocol, userfriendly, status);
		fAccount->Add(info->ID(), info);
		
		fTipText << "\n  " << info->DisplayLabel() << ": " << _T(info->StatusLabel()) << "";

		if ((fStatus > kStatusOnline) && (info->Status() == Online)) fStatus = kStatusOnline;
		if ((fStatus > kStatusAway) && (info->Status() == Away)) fStatus = kStatusAway;
	}

	switch (fStatus) {
		case kStatusOnline: {
			fCurrIcon = fModeIcon = fOnlineIcon;
		} break;
		case kStatusAway: {
			fCurrIcon = fModeIcon = fAwayIcon;
		} break;
		default: {
			fCurrIcon = fModeIcon =  fOfflineIcon;
		};
	};
};

void IM_DeskbarIcon::Draw(BRect /*rect*/) {
	SetHighColor( Parent()->ViewColor() );
	FillRect( Bounds() );
	
	if (fCurrIcon != NULL) {
		DrawBitmap(fCurrIcon, BPoint(0,0));
	} else {
		SetHighColor(255,0,0);
		FillRect( Bounds() );
	};
};

status_t IM_DeskbarIcon::Archive(BMessage * msg, bool deep) const {
	LOG(kLogName, liHigh, "IM_DeskbarIcon::Archive()");
	
	status_t res = BView::Archive(msg,deep);
	
	msg->AddString("add_on", DESKBAR_ICON_SIG );
	msg->AddString("add-on", DESKBAR_ICON_SIG );
	
	msg->AddString("class", "IM_DeskbarIcon");
	
	return res;
};

void IM_DeskbarIcon::MessageReceived(BMessage * msg) {
	switch (msg->what) {
		case IM::SETTINGS_UPDATED: {
			// settings have been updated, reload from im_server
			reloadSettings();
		} break;
		
		case 'blnk': {
			// blink icon
			BBitmap *oldIcon = fCurrIcon;
			
			if ((fFlashCount > 0) && ((fBlink++ % 2) || !fShouldBlink)) {
				fCurrIcon = fGenericIcon;
			} else {
				fCurrIcon = fModeIcon;
			};
			
			if (oldIcon != fCurrIcon) {
				Invalidate();
			};
		} break;
		
		case IM::FLASH_DESKBAR: {
			BMessenger msgr;
			if (msg->FindMessenger("messenger", &msgr) == B_OK) {
				fMsgrs.push_back(msgr);
			};
			
			fFlashCount++;
			fBlink = 0;
			
			if (fMsgRunner == NULL)	{
				BMessage msg('blnk');
				fMsgRunner = new BMessageRunner(BMessenger(this), &msg, 200 * 1000);
			};
			
			LOG(kLogName, liDebug, "IM: fFlashCount: %ld", fFlashCount);
		} break;
		case IM::STOP_FLASHING: {	
			BMessenger msgr;
			if (msg->FindMessenger("messenger", &msgr) == B_OK) {
				fMsgrs.remove(msgr);
			};
			
			fFlashCount--;
			LOG(kLogName, liDebug, "IM: fFlashCount: %ld", fFlashCount);
			
			if (fFlashCount == 0) {
				if (fMsgRunner != NULL) delete fMsgRunner;
				fMsgRunner = NULL;
				fCurrIcon = fModeIcon;
				Invalidate();
			};
			
			if (fFlashCount < 0) {
				fFlashCount = 0;
				LOG(kLogName, liMedium, "IM: fFlashCount below zero, fixing");
			};
		} break;
		
		case kMsgSetStatus: {
			const char *status = NULL;
			const char *protocol = NULL;
			
			if (msg->FindString("status", &status) != B_OK) {
				LOG(kLogName, liDebug, "No 'status' in kMSgSetStatus message", msg);
				return;
			};

			if (msg->FindString("protocol", &protocol) != B_OK) {
				protocol = NULL;
			};
			
			if (strcmp(AWAY_TEXT, status) == 0) {
				AwayMessageWindow *w = new AwayMessageWindow(protocol);
				w->Show();
				return;
			}
			
			BMessage newmsg(IM::MESSAGE);
			newmsg.AddInt32("im_what", IM::SET_STATUS);
			
			if (protocol != NULL) {
				newmsg.AddString("protocol", protocol);
			};
			
			newmsg.AddString("status", status);
			
			fCurrIcon = fModeIcon; 
			Invalidate();
			
			IM::Manager man;
			man.SendMessage(&newmsg);
		} break;
		
		case kMsgCloseIMServer: {
			BMessenger msgr(IM_SERVER_SIG);
			msgr.SendMessage(B_QUIT_REQUESTED);
		} break;
		
		case kMsgStartIMServer: {
			be_roster->Launch(IM_SERVER_SIG);
		} break;
		
		case kMsgOpenSettings: {
			be_roster->Launch(PREFLET_SIGNATURE);
		} break;
		
		case IM::MESSAGE: {
			int32 im_what;
			msg->FindInt32("im_what", &im_what);
			LOG(kLogName, liLow, "Got IM what of %i", im_what);
			
			switch (im_what) {
				case IM::STATUS_SET: {			
					fDirtyStatus = true;
					fDirtyStatusMenu = true;
				
					const char *status = msg->FindString("total_status");
								
					LOG(kLogName, liMedium, "Status set to %s", status);
					if (strcmp(status, ONLINE_TEXT) == 0) {
						fStatus = kStatusOnline;
						fModeIcon = fOnlineIcon;
					};
					if (strcmp(status, AWAY_TEXT) == 0) {
						fStatus = kStatusAway;
						fModeIcon = fAwayIcon;
					};
					if (strcmp(status, OFFLINE_TEXT) == 0) {
						fStatus = kStatusOffline;
						fModeIcon = fOfflineIcon;
					};
					
					fCurrIcon = fModeIcon;
					Invalidate();
				} break; 
			};
		} break;

		case B_NODE_MONITOR: {
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK) {
				switch (opcode) {
					case B_ENTRY_CREATED: {
						AddQueryRef(msg);
					} break;
					case B_ENTRY_MOVED: {
						BPath queryPath;
						BDirectory queryDir;
						node_ref querydir_ref;
						find_directory(B_USER_SETTINGS_DIRECTORY, &queryPath, true);
						queryPath.Append("im_kit/queries");

						queryDir = BDirectory(queryPath.Path());
						queryDir.GetNodeRef(&querydir_ref);
						
						int64 nodeFrom = msg->FindInt64("from directory");
						int64 nodeTo = msg->FindInt64("to directory");
						if (nodeFrom == querydir_ref.node) {
							if (nodeTo == querydir_ref.node) {
//								Entry is renamed. Lets take it easy. Remove the old
//								entry and then add it.
								RemoveQueryRef(msg);
								AddQueryRef(msg);
							} else {
								RemoveQueryRef(msg);
							};
						} else {
							AddQueryRef(msg);
						};
					
					} break;
					case B_ENTRY_REMOVED: {
						RemoveQueryRef(msg);
					} break;
				};
			};
		} break;
		
		case kMsgQueryUpdated: {
		} break;
		
		case kMsgLaunchFile: {
			entry_ref fileRef;
			msg->FindRef("fileRef", &fileRef);
			
			const char *handler = "application/x-vnd.Be-TRAK";
			int32 length = 0;
			char *mime = ReadAttribute(BNode(&fileRef), "BEOS:TYPE", &length);
			mime = (char *)realloc(mime, (length + 1) * sizeof(char));
			mime[length] = '\0';
			
			if (strcmp(mime, "application/x-person") == 0) handler = fPeopleApp;
			free(mime);
			
			BMessage openMsg(B_REFS_RECEIVED);
			openMsg.AddRef("refs", &fileRef);
			
			be_roster->Launch(handler, &openMsg);
			
		} break;
		
		case kMsgOpenQuery: {
//			For great justice, take off every query!
			entry_ref queryRef;
			msg->FindRef("queryRef", &queryRef);

			BMessage open_msg(B_REFS_RECEIVED);
			open_msg.AddRef("refs", &queryRef);
			
			be_roster->Launch("application/x-vnd.Be-TRAK", &open_msg);
		} break;

		case kMsgOpenQueryDir: {
			BPath queryPath;
			entry_ref ref;
			
			find_directory(B_USER_SETTINGS_DIRECTORY, &queryPath, true);
			queryPath.Append("im_kit/queries");

			if (get_ref_for_path(queryPath.Path(), &ref) == B_OK) {
				BMessage open(B_REFS_RECEIVED);
				open.AddRef("refs", &ref);
				
				be_roster->Launch("application/x-vnd.Be-TRAK", &open);
			};
		
		} break;
		
		case B_ABOUT_REQUESTED: {
			AboutRequested();
		} break;
		
		case B_SOME_APP_LAUNCHED: {
			const char * sig = NULL;
			if ( msg->FindString("be:signature", &sig) == B_OK ) {
				if ( strcmp(sig, IM_SERVER_SIG) == 0 ) {
					// register with im_server
					LOG(kLogName, liDebug, "Registering with im_server");
					BMessage msg(IM::REGISTER_DESKBAR_MESSENGER);
					msg.AddMessenger( "msgr", BMessenger(this) );
					
					BMessenger(IM_SERVER_SIG).SendMessage(&msg);
				}
			}
		} break;
		
		case IM::LOADED_PROTOCOLS_CHANGED: {
			// Rebuild the protocol menu
			fDirtyStatusMenu = true;
			LOG(kLogName, liDebug, "Received IM::LOADED_PROTOCOLS_CHANGED");
		} break;
		
		default: { 
			BView::MessageReceived(msg);
		} break;
	}
}

void IM_DeskbarIcon::MouseMoved(BPoint /*point*/, uint32 transit, const BMessage */*msg*/) {
	if ( transit == B_ENTERED_VIEW )
//	if ( (transit != B_OUTSIDE_VIEW) && (transit != B_EXITED_VIEW) )
	{ // update bubblehelper text and fetch statuses if needed
		if ( !BMessenger(IM_SERVER_SIG).IsValid() )
		{
			fTipText = _T("im_server not running");
		} else 
		{
			IM::Manager man;

//			XXX			
//			if ((fDirtyStatus == true) || (fStatuses.size() == 0)) {
//				fStatuses.clear();
			if ((fDirtyStatus == true) || (fAccount->CountItems() == 0)) {
				getProtocolStates(); // updates tip text
				fDirtyStatus = false;
			}
		}
		
		fTip->SetHelp(Parent(), (char *)fTipText.String());
		fTip->EnableHelp();
	}

	if ((transit == B_OUTSIDE_VIEW) || (transit == B_EXITED_VIEW)) {
		fTip->SetHelp(Parent(), NULL);
	}
}


void IM_DeskbarIcon::MouseDown(BPoint p) {
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	
	if (buttons & B_SECONDARY_MOUSE_BUTTON) {
		fMenu = new BPopUpMenu("im_db_menu", false, false);
		fMenu->SetFont(be_plain_font);
		
		if (BMessenger(IM_SERVER_SIG).IsValid() == false) {
			// im_server not running!
			fStatus = kStatusOffline;
			fModeIcon = fOfflineIcon;
			
			LOG(kLogName, liDebug, "Build menu: im_server not running");
			fMenu->AddItem( new BMenuItem(_T("Start im_server"), new BMessage(kMsgStartIMServer)) );
			
			fMenu->SetTargetForItems( this );

			ConvertToScreen(&p);
			BRect r(p, p);
			r.InsetBySelf(-2, -2);
			
			fMenu->Go(p, true, true, r, true);
			
			fMenu->SetAsyncAutoDestruct(true); // delete fMenu;
			
			return;
		};
				
		if ( true || fDirtyStatusMenu || !fStatusMenu) {
			LOG(kLogName, liLow, "Rebuilding status menu");
			fStatusMenu = new BMenu(_T("Set status"));
			
			BMenu *total = new BMenu(_T("All Accounts"));
			BMessage *msg_online = new BMessage(kMsgSetStatus); msg_online->AddString("status", ONLINE_TEXT);
			total->AddItem(new BMenuItem(_T(ONLINE_TEXT), msg_online) );	
			BMessage *msg_away = new BMessage(kMsgSetStatus); msg_away->AddString("status", AWAY_TEXT);
			total->AddItem(new BMenuItem(_T(AWAY_TEXT), msg_away) );	
			BMessage *msg_offline = new BMessage(kMsgSetStatus); msg_offline->AddString("status", OFFLINE_TEXT);
			total->AddItem(new BMenuItem(_T(OFFLINE_TEXT), msg_offline) );	
			
			total->ItemAt(fStatus)->SetMarked(true);
			total->SetTargetForItems(this);
			total->SetFont(be_plain_font);
			
			fStatusMenu->AddItem(total);
			fStatusMenu->AddSeparatorItem();
			
			fStatusMenu->SetFont(be_plain_font);
					
			getProtocolStates();
					
			for (AccountStore::Iterator it = fAccount->Start(); it != fAccount->End(); it++) {
				AccountInfo *info = it->second;
								
				BString name = info->DisplayLabel();

				// Message template
				BMessage accountMsg(kMsgSetStatus);
				accountMsg.AddString("account", info->ID());
				accountMsg.AddString("protocol", info->Protocol());
				
				BMenu *account = new BMenu(name.String());
								
				// Online
				BMessage *onlineMsg = new BMessage(accountMsg);
				onlineMsg->AddString("status", ONLINE_TEXT);
				account->AddItem(new BMenuItem(_T(ONLINE_TEXT), onlineMsg));

				// Away
				BMessage *awayMsg = new BMessage(accountMsg);
				awayMsg->AddString("status", AWAY_TEXT);
				account->AddItem(new BMenuItem(_T(AWAY_TEXT), awayMsg));
				
				// Offline
				BMessage *offlineMsg = new BMessage(accountMsg);
				offlineMsg->AddString("status", OFFLINE_TEXT);
				account->AddItem(new BMenuItem(_T(OFFLINE_TEXT), offlineMsg));
				
				account->SetTargetForItems(this);
				switch (info->Status()) {
					case Online: {
						account->ItemAt(0)->SetMarked(true);
					} break;
					case Away: {
						account->ItemAt(1)->SetMarked(true);
					} break;
					case Offline: {
						account->ItemAt(2)->SetMarked(true);
					} break;
				};
				
				account->SetFont(be_plain_font);
				fStatusMenu->AddItem(account);
			};

			fStatusMenu->SetTargetForItems(this);
			
			fDirtyStatusMenu = false;
		};
		
		fMenu->AddItem(fStatusMenu);

		fMenu->AddSeparatorItem();

//		Queries
		BuildQueryMenu();
		fMenu->AddItem(fQueryMenu);

//		settings
		fMenu->AddItem( new BMenuItem(_T("Settings"), new BMessage(kMsgOpenSettings)) );

//		About
		fMenu->AddItem(new BMenuItem(_T("About"), new BMessage(B_ABOUT_REQUESTED)));
	
//		Quit
		fMenu->AddSeparatorItem();
		fMenu->AddItem(new BMenuItem(_T("Quit"), new BMessage(kMsgCloseIMServer)));
	
		fMenu->SetTargetForItems( this );

		ConvertToScreen(&p);
		BRect r(p, p);
		r.InsetBySelf(-2, -2);
		
		fMenu->Go(p, true, true, r, true);
		
		fMenu->SetAsyncAutoDestruct(true); // delete fMenu;
		
		return;
	};

	if ((buttons & B_TERTIARY_MOUSE_BUTTON) ||
		((modifiers() & B_COMMAND_KEY) & (buttons & B_PRIMARY_MOUSE_BUTTON))) {
		
		BMessage openPeople(B_REFS_RECEIVED);
		openPeople.AddRef("refs", &fMiddleClickRef);
		
		BMessenger tracker("application/x-vnd.Be-TRAK");
		tracker.SendMessage(&openPeople);
		
		return;
	}

	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		list<BMessenger>::iterator i = fMsgrs.begin();
		
		if (i != fMsgrs.end()) {
			(*i).SendMessage( IM::DESKBAR_ICON_CLICKED );
		};
		
		return;
	};
}

void
IM_DeskbarIcon::AttachedToWindow()
{
	LOG(kLogName, liHigh, "IM_DeskbarIcon::AttachedToWindow()");

	// give im_server a chance to start up
	snooze(500*1000);
	reloadSettings();
	
	// register with im_server
	LOG(kLogName, liDebug, "Registering with im_server");
	BMessage msg(IM::REGISTER_DESKBAR_MESSENGER);
	msg.AddMessenger( "msgr", BMessenger(this) );
	
	BMessenger(IM_SERVER_SIG).SendMessage(&msg);
	
	// Start listening to app launches and quits
	if ( be_roster->StartWatching( BMessenger(this) ) != B_OK )
	{
		LOG(kLogName, liHigh, "Couldn't start watching app launches");
	}
	
	BVolumeRoster volRoster;
	BVolume bootVol;
	BMessenger target(this, NULL, NULL);
//	querymap::iterator qIt;
	BPath queryPath;
	entry_ref queryRef;

	find_directory(B_USER_SETTINGS_DIRECTORY, &queryPath, true);
	queryPath.Append("im_kit/queries");

	BDirectory queryDir(queryPath.Path());
	queryDir.Rewind();
	volRoster.GetBootVolume(&bootVol);


	node_ref nref;
	if (queryDir.InitCheck() == B_OK) {
		queryDir.GetNodeRef(&nref);
		watch_node(&nref, B_WATCH_DIRECTORY, target);
	};
	
	fQueryMenu = new BMenu("Queries");
	fQueryMenu->AddItem(new BMenuItem("Open Query directory", new BMessage(kMsgOpenQueryDir)));
	fQueryMenu->AddSeparatorItem();
	
	for (int32 i = 0; queryDir.GetNextRef(&queryRef) == B_OK; i++) {
		BNode node(&queryRef);
		queryinfo info;
		BString label = queryRef.name;
		
		info.ref = queryRef;
		info.icon = ReadNodeIcon(BPath(&queryRef).Path());
		node.GetNodeRef(&info.nref);

//		Query stuff
		int32 length = 0;
		vollist volumes;
		char *predicate = ReadAttribute(node, kTrackerQueryPredicate, &length);
		predicate = (char *)realloc(predicate, sizeof(char) * (length + 1));
		predicate[length] = '\0';

		if (ExtractVolumes(&node, &volumes) == B_OK) {
			BMessage *msg = new BMessage(kMsgQueryUpdated);
			msg->AddRef("ref", &info.ref);
			info.query = new QueryLooper(predicate, volumes, info.ref.name, this, msg);
			
			label << " (" << info.query->CountEntries() << ")";
		} else {
			info.query = NULL;
		};

		fQueries[queryRef] = info;
		
		BMessage *queryMsg = new BMessage(kMsgOpenQuery);
		queryMsg->AddRef("queryRef", &queryRef);
		
		IconMenuItem *item = new IconMenuItem(info.icon, label.String(), "Empty",
			queryMsg, false);
		fQueryMenu->AddItem(item);
	};

	fQueryMenu->SetFont(be_plain_font);
	fQueryMenu->SetTargetForItems(this);
}

void IM_DeskbarIcon::DetachedFromWindow() {
	LOG(kLogName, liHigh, "IM_DeskbarIcon::DetachedFromWindow()");
	
	querymap::iterator qIt;
	
	for (qIt = fQueries.begin(); qIt != fQueries.end(); qIt++) {
		queryinfo info = qIt->second;
		
		delete info.icon;
		if (info.query) {
			BMessenger(info.query).SendMessage(B_QUIT_REQUESTED);
		};
	};
}

void IM_DeskbarIcon::AboutRequested(void) {
	BString text = "";
	text << _T("SVN Version: ") << BUILD_REVISION << "\n"
		<< _T("Built on: " ) << BUILD_DATE << "\n\n"
		<< _T("The IM Kit is a collaborative effort released under the OsDrawer.net banner.")
		<< _T(" Collaborators include (in alphabetical order):" ) << "\n"
		<< "  - Bruno G. \"bga\" Albuquerque\n"
		<< "  - Andrea \"xeD\" Anzani\n"
		<< "  - Michael \"slaad\" Davidson\n"
		<< "  - Mikael \"m_eiman\" Eiman\n"
		<< "  - Pier Luigi \"plfiorini\" Fiorini\n"
		<< "  - Oliver \"OliverH\" Hartmann\n"
		<< "  - Mikael \"tic\" Jansson\n";

	BAlert *alert = new BAlert("About",
		text.String(), "Wow!", NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING,
		B_INFO_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	alert->SetFeel(B_NORMAL_WINDOW_FEEL);
	alert->Go(NULL);
	
};

void
IM_DeskbarIcon::reloadSettings()
{
	LOG(kLogName, liLow, "IM: Requesting settings");
	
	BMessage settings;
	
	im_load_client_settings("im_server", &settings );

	settings.FindBool("blink_db", &fShouldBlink );
			
	if (settings.FindString("person_handler", &fPeopleApp) != B_OK) {
		fPeopleApp = "application/x-vnd.Be-TRAK";
	};
			
	LOG(kLogName, liMedium, "IM: Settings applied");
}

//#pragma mark -

void IM_DeskbarIcon::BuildQueryMenu(void) {
	querymap::iterator qIt;
	fQueryMenu = new BMenu("Queries");
	
	fQueryMenu->AddItem(new BMenuItem(_T("Open Query directory"), new BMessage(kMsgOpenQueryDir)));
	fQueryMenu->AddSeparatorItem();
	
	for (qIt = fQueries.begin(); qIt != fQueries.end(); qIt++) {
		queryinfo info = qIt->second;
		BString label = info.ref.name;
		if (info.query) {
			label << " (" << info.query->CountEntries() << ")";
		};

		BMessage *queryMsg = new BMessage(kMsgOpenQuery);
		queryMsg->AddRef("queryRef", &info.ref);
		
		IconMenuItem *item = new IconMenuItem(info.icon, label.String(), "Empty",
			queryMsg, false);
		fQueryMenu->AddItem(item);
	};
	
	fQueryMenu->SetFont(be_plain_font);
	fQueryMenu->SetTargetForItems(this);
};

void IM_DeskbarIcon::AddQueryRef(BMessage *msg) {
// 	XXX: For some reason the name isn't getting set in the
//	entry_ref, so the icon won't be right
	queryinfo info;
	const char *nameTemp;
	char *name = NULL;
	int32 length = 0;
	
	msg->FindInt32("device", &info.ref.device);
	msg->FindInt64("directory", &info.ref.directory);
	msg->FindString("name", &nameTemp);
	
	length = strlen(nameTemp);
	name = (char *)calloc(length + 1, sizeof(char));
	memcpy(name, nameTemp, length);
	name[length] = '\0';
	
	info.ref.set_name(name);
	free(name);

//	Query stuff
	BNode node(&info.ref);
	vollist volumes;
	char *predicate = ReadAttribute(node, kTrackerQueryPredicate, &length);
	predicate = (char *)realloc(predicate, sizeof(char) * (length + 1));
	predicate[length] = '\0';

	if (ExtractVolumes(&node, &volumes) == B_OK) {
		BMessage *msg = new BMessage(kMsgQueryUpdated);
		msg->AddRef("ref", &info.ref);
		info.query = new QueryLooper(predicate, volumes, info.ref.name, this, msg);
	} else {
		info.query = NULL;
	};
	free(predicate);
	
	msg->FindInt32("device", &info.nref.device);
	msg->FindInt64("node", &info.nref.node);

	info.icon = ReadNodeIcon(BPath(&info.ref).Path());

	fQueries[info.ref] = info;
};

void IM_DeskbarIcon::RemoveQueryRef(BMessage *msg) {
	querymap::iterator qIt;
	node_ref nref;
	
	msg->FindInt32("device", &nref.device);
	msg->FindInt64("node", &nref.node);
						
	for (qIt = fQueries.begin(); qIt != fQueries.end(); qIt++) {
		queryinfo info = qIt->second;
		if (info.nref == nref) {
			fQueries.erase(info.ref);
			if (info.query) {
				BMessenger(info.query).SendMessage(B_QUIT_REQUESTED);
			}
			delete info.icon;
			break;
		};
	};
	
	return;
};

status_t IM_DeskbarIcon::ExtractVolumes(BNode *node, vollist *volumes) {
	int32 length = 0;
	char *attr = ReadAttribute(*node, kTrackerQueryVolume, &length);
	BVolumeRoster roster;

	if (attr == NULL) {
		roster.Rewind();
		BVolume vol;
		
		while (roster.GetNextVolume(&vol) == B_NO_ERROR) {
			if (vol.KnowsQuery() == true) volumes->push_back(vol);
		};
	} else {
		BMessage msg;
		msg.Unflatten(attr);

//		!*YOINK*!d from that project... with the funny little doggie as a logo...
//		OpenTracker, that's it!
			
		time_t created;
		off_t capacity;
		
		for (int32 index = 0; msg.FindInt32("creationDate", index, &created) == B_OK;
			index++) {
			
			if ((msg.FindInt32("creationDate", index, &created) != B_OK)
				|| (msg.FindInt64("capacity", index, &capacity) != B_OK))
				return B_ERROR;
		
			BVolume volume;
			BString deviceName = "";
			BString volumeName = "";
			BString fshName = "";
		
			if (msg.FindString("deviceName", &deviceName) == B_OK
				&& msg.FindString("volumeName", &volumeName) == B_OK
				&& msg.FindString("fshName", &fshName) == B_OK) {
				// New style volume identifiers: We have a couple of characteristics,
				// and compute a score from them. The volume with the greatest score
				// (if over a certain threshold) is the one we're looking for. We
				// pick the first volume, in case there is more than one with the
				// same score.
				int foundScore = -1;
				roster.Rewind();
				while (roster.GetNextVolume(&volume) == B_OK) {
					if (volume.IsPersistent() && volume.KnowsQuery()) {
						// get creation time and fs_info
						BDirectory root;
						volume.GetRootDirectory(&root);
						time_t cmpCreated;
						fs_info info;
						if (root.GetCreationTime(&cmpCreated) == B_OK
							&& fs_stat_dev(volume.Device(), &info) == 0) {
							// compute the score
							int score = 0;
		
							// creation time
							if (created == cmpCreated)
								score += 5;
							// capacity
							if (capacity == volume.Capacity())
								score += 4;
							// device name
							if (deviceName == info.device_name)
								score += 3;
							// volume name
							if (volumeName == info.volume_name)
								score += 2;
							// fsh name
							if (fshName == info.fsh_name)
								score += 1;
		
							// check score
							if (score >= 9 && score > foundScore) {
								volumes->push_back(volume);
							}
						}
					}
				}
			} else {
				// Old style volume identifiers: We have only creation time and
				// capacity. Both must match.
				roster.Rewind();
				while (roster.GetNextVolume(&volume) == B_OK)
					if (volume.IsPersistent() && volume.KnowsQuery()) {
						BDirectory root;
						volume.GetRootDirectory(&root);
						time_t cmpCreated;
						root.GetCreationTime(&cmpCreated);
						if (created == cmpCreated && capacity == volume.Capacity()) {
							volumes->push_back(volume);
						}
					}
			}
		};
	};

	return B_OK;	
};
