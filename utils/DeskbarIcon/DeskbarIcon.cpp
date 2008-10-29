#include "DeskbarIcon.h"

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>
#include <Message.h>
#include <stdio.h>
#include <Roster.h>
#include <File.h>
#include <MenuItem.h>
#include <Application.h>
#include <Window.h>
#include <Deskbar.h>

#include <Path.h>
#include <FindDirectory.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

const char *kTrackerQueryVolume = "_trk/qryvol1";
const char *kTrackerQueryPredicate = "_trk/qrystr";

BView *
instantiate_deskbar_item()
{
	LOG("deskbar", liHigh, "IM: Instantiating Deskbar item");
	return new IM_DeskbarIcon();
}

//#pragma mark -

BArchivable *
IM_DeskbarIcon::Instantiate( BMessage * archive )
{
	if ( !validate_instantiation(archive,"IM_DeskbarIcon") )
	{
		LOG("deskbar", liHigh, "IM_DeskbarIcon::Instantiate(): Invalid archive");
		return NULL;
	}
	
	LOG("deskbar", liHigh, "IM_DeskbarIcon::Instantiate() ok");
	
	return new IM_DeskbarIcon(archive);
}


IM_DeskbarIcon::IM_DeskbarIcon()
	: BView(BRect(0,0,15,15), DESKBAR_ICON_NAME, B_FOLLOW_NONE, B_WILL_DRAW) {
	LOG("deskbar", liHigh, "IM_DeskbarIcon::IM_DeskbarIcon()");

	_init();
}

IM_DeskbarIcon::IM_DeskbarIcon(BMessage * archive)
	: BView(archive) {
	LOG("deskbar", liHigh, "IM_DeskbarIcon::IM_DeskbarIcon(BMessage*)");

	_init();
}

IM_DeskbarIcon::~IM_DeskbarIcon() {
	LOG("deskbar", liHigh, "IM_DeskbarIcon::~IM_DeskbarIcon()");

	delete fTip;
	delete fAwayIcon;
	delete fOnlineIcon;
	delete fOfflineIcon;
	delete fFlashIcon;
	if ( fMsgRunner )
		delete fMsgRunner;
}

void
IM_DeskbarIcon::_init() {
	LOG("deskbar", liHigh, "IM_DeskbarIcon::_init()");

	BPath userDir;
	find_directory(B_USER_SETTINGS_DIRECTORY, &userDir, true);
	
	BPath iconDir = userDir;
	iconDir.Append("im_kit/icons");
	
//	Load the Offline, Away, Online and Flash icons from disk
	BString iconPath = iconDir.Path();
	iconPath << "/DeskbarAway";
	fAwayIcon = GetBitmapFromAttribute(iconPath.String(), BEOS_MINI_ICON_ATTRIBUTE,
		'ICON');
	if ( !fAwayIcon )
		LOG("deskbar", liHigh, "Error loading fAwayIcon");
	
	iconPath = iconDir.Path();
	iconPath << "/DeskbarOnline";
	fOnlineIcon = GetBitmapFromAttribute(iconPath.String(), BEOS_MINI_ICON_ATTRIBUTE,
		'ICON');
	if ( !fOnlineIcon )
		LOG("deskbar", liHigh, "Error loading fOnlineIcon");
	
		
	iconPath = iconDir.Path();
	iconPath << "/DeskbarOffline";
	fOfflineIcon = GetBitmapFromAttribute(iconPath.String(), BEOS_MINI_ICON_ATTRIBUTE,
		'ICON');
	if ( !fOfflineIcon )
		LOG("deskbar", liHigh, "Error loading fOfflineIcon");
	

	iconPath = iconDir.Path();
	iconPath << "/DeskbarFlash";
	fFlashIcon = GetBitmapFromAttribute(iconPath.String(), BEOS_MINI_ICON_ATTRIBUTE,
		'ICON');
	if ( !fFlashIcon )
		LOG("deskbar", liHigh, "Error loading fFlashIcon");
	

//	Initial icon is the Offline icon
	fCurrIcon = fModeIcon = fOfflineIcon;
	fStatus = 2;

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
	if( path.InitCheck() == B_OK )
		be_locale.LoadLanguageFile( path.Path() );
#endif
}

void
IM_DeskbarIcon::getProtocolStates()
{
	BMessage protStatus;
	fStatuses.clear();
	IM::Manager man;
	if ( man.SendMessage(new BMessage(IM::GET_OWN_STATUSES), &protStatus) != B_OK )
	{
		LOG("deskbar", liHigh, "Error getting statuses");
	}

	fTipText = "Online Status:";
	
	for ( int i=0; protStatus.FindString("protocol",i); i++ ) {
		const char *protocol = protStatus.FindString("protocol",i);
		const char *userfriendly = protStatus.FindString("userfriendly",i);
		const char *status = protStatus.FindString("status", i);
		
		fStatuses[protocol] = status;
		fFriendlyNames[protocol] = userfriendly;
		
		fTipText << "\n  " << userfriendly << ": " << _T(status) << "";
		
		if ((fStatus > 0) && (strcmp(status, ONLINE_TEXT) == 0)) fStatus = 0;
		if ((fStatus > 1) && (strcmp(status, AWAY_TEXT) == 0)) fStatus = 1;
	}

	LOG("deskbar", liDebug, "Initial status: %i	", fStatus);
	
	switch (fStatus) {
//		Online
		case 0: {
			fCurrIcon = fModeIcon = fOnlineIcon;
		} break;
//		Away
		case 1: {
			fCurrIcon = fModeIcon = fAwayIcon;
		} break;
		default: {
			fCurrIcon = fModeIcon =  fOfflineIcon;
		};
	};
}

void
IM_DeskbarIcon::Draw( BRect /*rect*/ )
{
	SetHighColor( Parent()->ViewColor() );
	FillRect( Bounds() );
	
	if ( fCurrIcon )
	{
		DrawBitmap( fCurrIcon, BPoint(0,0) );
	} else
	{
		SetHighColor(255,0,0);
		FillRect( Bounds() );
	}
}

status_t
IM_DeskbarIcon::Archive( BMessage * msg, bool deep ) const
{
	LOG("deskbar", liHigh, "IM_DeskbarIcon::Archive()");
	
	status_t res = BView::Archive(msg,deep);
	
	msg->AddString("add_on", DESKBAR_ICON_SIG );
	msg->AddString("add-on", DESKBAR_ICON_SIG );
	
	msg->AddString("class", "IM_DeskbarIcon");
	
	return res;
}

void
IM_DeskbarIcon::MessageReceived( BMessage * msg )
{	
	switch ( msg->what )
	{
		case IM::SETTINGS_UPDATED:
		{ // settings have been updated, reload from im_server
			reloadSettings();
		}	break;
		
		case 'blnk':
		{ // blink icon
			BBitmap * oldIcon = fCurrIcon;
			
			if ( (fFlashCount > 0) && ((fBlink++ % 2) || !fShouldBlink))
			{
				fCurrIcon = fFlashIcon;
			} else
			{
				fCurrIcon = fModeIcon;
			}
			
			if ( oldIcon != fCurrIcon )
				Invalidate();
		}	break;
		
		case IM::FLASH_DESKBAR:
		{
			BMessenger msgr;
			if ( msg->FindMessenger("messenger", &msgr) == B_OK )
			{
				fMsgrs.push_back( msgr );
			}
			
			fFlashCount++;
			fBlink = 0;
			if ( !fMsgRunner )
			{
				BMessage msg('blnk');
				fMsgRunner = new BMessageRunner( BMessenger(this), &msg, 200*1000 );
			}
			LOG("deskbar", liDebug, "IM: fFlashCount: %ld", fFlashCount);
		}	break;
		case IM::STOP_FLASHING:
		{	
			BMessenger msgr;
			if ( msg->FindMessenger("messenger", &msgr) == B_OK )
			{
				fMsgrs.remove( msgr );
			}
			
			fFlashCount--;
			LOG("deskbar", liDebug, "IM: fFlashCount: %ld", fFlashCount);
			
			if ( fFlashCount == 0 )
			{
				if ( fMsgRunner ) delete fMsgRunner;
				fMsgRunner = NULL;
				fCurrIcon = fModeIcon;
				Invalidate();
			}
			
			if ( fFlashCount < 0 )
			{
				fFlashCount = 0;
				LOG("deskbar", liMedium, "IM: fFlashCount below zero, fixing");
			}
		}	break;
		
		case SET_STATUS:
		{
//			BMenuItem *item = NULL;

			const char *protocol = msg->FindString("protocol");
			const char *status = msg->FindString("status");
			
			if ( !status ) {
				LOG("deskbar", liDebug, "No 'status' in SET_STATUS message", msg);
				return;
			}
			
			if (strcmp(AWAY_TEXT, status) == 0) {
				AwayMessageWindow *w = new AwayMessageWindow(protocol);
				w->Show();
				return;
			}
			
			BMessage newmsg(IM::MESSAGE);
			newmsg.AddInt32("im_what", IM::SET_STATUS);
			
			if ( protocol != NULL ) newmsg.AddString("protocol", protocol);
			
			newmsg.AddString("status", status);
			
			fCurrIcon = fModeIcon; 
			Invalidate();
			
			IM::Manager man;
			man.SendMessage(&newmsg);
		}	break;
		
		case CLOSE_IM_SERVER: {
			LOG("deskbar", liHigh, "Got Quit message");
			BMessenger msgr(IM_SERVER_SIG);
			msgr.SendMessage(B_QUIT_REQUESTED);
		} break;
		
		case START_IM_SERVER: {
			be_roster->Launch(IM_SERVER_SIG);
		} break;
		
		case OPEN_SETTINGS:
		{
			be_roster->Launch("application/x-vnd.beclan-IMKitPrefs");
		}	break;
		
		case IM::MESSAGE: {
			int32 im_what;
			msg->FindInt32("im_what", &im_what);
			LOG("deskbar", liLow, "Got IM what of %i", im_what);
			
			switch (im_what) {
				case IM::STATUS_SET: {			
					fDirtyStatus = true;
					fDirtyStatusMenu = true;
				
					const char *status = msg->FindString("total_status");
								
					LOG("deskbar", liMedium, "Status set to %s", status);
					if (strcmp(status, ONLINE_TEXT) == 0) {
						fStatus = 0;
						fModeIcon = fOnlineIcon;
					}
					if (strcmp(status, AWAY_TEXT) == 0) {
						fStatus = 1;
						fModeIcon = fAwayIcon;
					};
					if (strcmp(status, OFFLINE_TEXT) == 0) {
						fStatus = 2;
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
				msg->PrintToStream();

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
		
		case QUERY_UPDATED: {
		} break;
		
		case LAUNCH_FILE: {
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
		
		case OPEN_QUERY: {
//			For great justice, take off every query!
			entry_ref queryRef;
			msg->FindRef("queryRef", &queryRef);

			BMessage open_msg(B_REFS_RECEIVED);
			open_msg.AddRef("refs", &queryRef);
			
			be_roster->Launch("application/x-vnd.Be-TRAK", &open_msg);
		} break;

		case OPEN_QUERY_DIR: {
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
//				LOG("deskbar", liDebug, "App started: %s", sig);
				if ( strcmp(sig, IM_SERVER_SIG) == 0 ) {
					// register with im_server
					LOG("deskbar", liDebug, "Registering with im_server");
					BMessage msg(IM::REGISTER_DESKBAR_MESSENGER);
					msg.AddMessenger( "msgr", BMessenger(this) );
					
					BMessenger(IM_SERVER_SIG).SendMessage(&msg);
				}
			}
		} break;
		
		default:
			BView::MessageReceived(msg);
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
			
			if ((fDirtyStatus == true) || (fStatuses.size() == 0)) {
				fStatuses.clear();
				
				getProtocolStates(); // updates tip text
				
/*				BMessage protStatus;
				man.SendMessage(new BMessage(IM::GET_OWN_STATUSES), &protStatus);
				
				fTipText = _T("Online Status:");
				for (int i = 0; protStatus.FindString("protocol",i); i++ ) {
					const char *protocol = protStatus.FindString("protocol",i);
					const char *status = protStatus.FindString("status", i);
	
					fStatuses[protocol] = status;
	
					fTipText << "\n  " << protocol << ": " << _T(status) << "";
					
					LOG("deskbar", liDebug, "Protocol status: %s is %s", protocol, status );
				}
*/				
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
		
		if ( !BMessenger(IM_SERVER_SIG).IsValid() )
		{ // im_server not running!
			fStatus = 2;
			fModeIcon = fOfflineIcon;
			
			LOG("deskbar", liDebug, "Build menu: im_server not running");
			fMenu->AddItem( new BMenuItem(_T("Start im_server"), new BMessage(START_IM_SERVER)) );
			
			fMenu->SetTargetForItems( this );

			ConvertToScreen(&p);
			BRect r(p, p);
			r.InsetBySelf(-2, -2);
			
			fMenu->Go(p, true, true, r, true);
			
			fMenu->SetAsyncAutoDestruct(true); // delete fMenu;
			
			return;
		}
		
		LOG("deskbar", liDebug, "Build menu: im_server running");
		
		if ( true || fDirtyStatusMenu || !fStatusMenu) {
			LOG("deskbar", liDebug, "Rebuilding status menu");
			fStatusMenu = new BMenu(_T("Set status"));
			
			LOG("deskbar", liDebug, "'All protocols'");
			BMenu *total = new BMenu(_T("All Protocols"));
			BMessage *msg_online = new BMessage(SET_STATUS); msg_online->AddString("status", ONLINE_TEXT);
			total->AddItem(new BMenuItem(_T(ONLINE_TEXT), msg_online) );	
			BMessage *msg_away = new BMessage(SET_STATUS); msg_away->AddString("status", AWAY_TEXT);
			total->AddItem(new BMenuItem(_T(AWAY_TEXT), msg_away) );	
			BMessage *msg_offline = new BMessage(SET_STATUS); msg_offline->AddString("status", OFFLINE_TEXT);
			total->AddItem(new BMenuItem(_T(OFFLINE_TEXT), msg_offline) );	
			
			total->ItemAt(fStatus)->SetMarked(true);
			total->SetTargetForItems(this);
			total->SetFont(be_plain_font);
			
			fStatusMenu->AddItem(total);
			fStatusMenu->AddSeparatorItem();
			
			fStatusMenu->SetFont(be_plain_font);
			
			map <string, string>::iterator it;
			
			getProtocolStates();
			
			LOG("deskbar", liDebug, "separate protocols");
			
			for (it = fStatuses.begin(); it != fStatuses.end(); it++) {
				string name = (*it).first;
				BMenu *protocol = new BMenu(fFriendlyNames[name].c_str());
				
				BMessage protMsg(SET_STATUS);
				protMsg.AddString("protocol", name.c_str());
				
				BMessage *msg1 = new BMessage(protMsg); msg1->AddString("status", ONLINE_TEXT);
				protocol->AddItem(new BMenuItem(_T(ONLINE_TEXT), msg1));
				BMessage *msg2 = new BMessage(protMsg); msg2->AddString("status", AWAY_TEXT);
				protocol->AddItem(new BMenuItem(_T(AWAY_TEXT), msg2));
				BMessage *msg3 = new BMessage(protMsg); msg3->AddString("status", OFFLINE_TEXT);
				protocol->AddItem(new BMenuItem(_T(OFFLINE_TEXT), msg3));
				
				protocol->SetTargetForItems(this);
				if ((*it).second == ONLINE_TEXT) {
					protocol->ItemAt(0)->SetMarked(true);
				} else if ((*it).second == AWAY_TEXT) {
					protocol->ItemAt(1)->SetMarked(true);
				} else {
					protocol->ItemAt(2)->SetMarked(true);
				};
				
				protocol->SetFont(be_plain_font);
				
				LOG("deskbar", liDebug, "+ %s (%s)", name.c_str(), (*it).second.c_str() );
				
				fStatusMenu->AddItem(protocol);
			};
			
			fStatusMenu->SetTargetForItems(this);
			
			fDirtyStatusMenu = false;
			LOG("deskbar", liDebug, "done rebuilding status menu");
		};
		
		fMenu->AddItem(fStatusMenu);

//		Queries
		BuildQueryMenu();
		fMenu->AddSeparatorItem();
		fMenu->AddItem(fQueryMenu);

//		settings
		fMenu->AddSeparatorItem();
		fMenu->AddItem( new BMenuItem(_T("Settings"), new BMessage(OPEN_SETTINGS)) );

//		About
		fMenu->AddSeparatorItem();
		fMenu->AddItem(new BMenuItem(_T("About"), new BMessage(B_ABOUT_REQUESTED)));
	
//		Quit
		fMenu->AddSeparatorItem();
		fMenu->AddItem(new BMenuItem(_T("Quit"), new BMessage(CLOSE_IM_SERVER)));
	
		fMenu->SetTargetForItems( this );

		LOG("deskbar", liDebug, "Done building, show");
		
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
	LOG("deskbar", liHigh, "IM_DeskbarIcon::AttachedToWindow()");

	// give im_server a chance to start up
	snooze(500*1000);
	reloadSettings();
	
	// register with im_server
	LOG("deskbar", liDebug, "Registering with im_server");
	BMessage msg(IM::REGISTER_DESKBAR_MESSENGER);
	msg.AddMessenger( "msgr", BMessenger(this) );
	
	BMessenger(IM_SERVER_SIG).SendMessage(&msg);
	
	// Start listening to app launches and quits
	if ( be_roster->StartWatching( BMessenger(this) ) != B_OK )
	{
		LOG("deskbar", liHigh, "Couldn't start watching app launches");
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
	fQueryMenu->AddItem(new BMenuItem("Open Query directory", new BMessage(OPEN_QUERY_DIR)));
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
			BMessage *msg = new BMessage(QUERY_UPDATED);
			msg->AddRef("ref", &info.ref);
			info.query = new QueryLooper(predicate, volumes, info.ref.name, this, msg);
			
			label << " (" << info.query->CountEntries() << ")";
		} else {
			info.query = NULL;
		};

		fQueries[queryRef] = info;
		
		BMessage *queryMsg = new BMessage(OPEN_QUERY);
		queryMsg->AddRef("queryRef", &queryRef);
		
		IconMenuItem *item = new IconMenuItem(info.icon, label.String(), "Empty",
			queryMsg, false);
		fQueryMenu->AddItem(item);
	};

	fQueryMenu->SetFont(be_plain_font);
	fQueryMenu->SetTargetForItems(this);
}

void IM_DeskbarIcon::DetachedFromWindow() {
	LOG("deskbar", liHigh, "IM_DeskbarIcon::DetachedFromWindow()");
	
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
		<< _T("The IM Kit is a collaborative effort released under the BeClan banner.")
		<< _T(" Collaborators include:" ) << "\n"
		<< "  - Mikael \"m_eiman\" Eiman\n"
		<< "  - Mikael \"tic\" Jansson\n"
		<< "  - B \"Lazy Sod ;)\" GA\n"
		<< "  - Andrea \"xeD\" Anzani\n"
		<< "  - Oliver \"OliverH\" Hartmann\n"
		<< "  - Michael \"slaad\" Davidson\n";

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
	LOG("deskbar", liLow, "IM: Requesting settings");
	
	BMessage settings;
	
	im_load_client_settings("im_server", &settings );

	settings.FindBool("blink_db", &fShouldBlink );
			
	if (settings.FindString("person_handler", &fPeopleApp) != B_OK) {
		fPeopleApp = "application/x-vnd.Be-TRAK";
	};
			
	LOG("deskbar", liMedium, "IM: Settings applied");
}

//#pragma mark -

void IM_DeskbarIcon::BuildQueryMenu(void) {
	querymap::iterator qIt;
	fQueryMenu = new BMenu("Queries");
	
	fQueryMenu->AddItem(new BMenuItem(_T("Open Query directory"), new BMessage(OPEN_QUERY_DIR)));
	fQueryMenu->AddSeparatorItem();
	
	for (qIt = fQueries.begin(); qIt != fQueries.end(); qIt++) {
		queryinfo info = qIt->second;
		BString label = info.ref.name;
		if (info.query) {
			label << " (" << info.query->CountEntries() << ")";
		};

		BMessage *queryMsg = new BMessage(OPEN_QUERY);
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
		BMessage *msg = new BMessage(QUERY_UPDATED);
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
