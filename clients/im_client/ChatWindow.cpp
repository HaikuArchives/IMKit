#include "ChatWindow.h"

#include "../../common/IMKitUtilities.h"
#include "../../common/IconView.h"

#include <libim/Connection.h>
#include <libim/Contact.h>
#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <Mime.h>
#include <Path.h>
#include <MenuBar.h>
#include <MessageRunner.h>
#include <vector>

#include <common/BubbleHelper.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

BubbleHelper gBubbles;

const char *kImNewMessageSound = "IM Message Received";
const float kPadding = 2.0;
const float kDockPadding = 12.0;
const int32 kTypingSendRate = 3 * 1000 * 1000;
const int32 kTypingStoppedRate = 5 * 1000 * 1000;
const char *kMobileDevice = "User is using a mobile device";

/*
#include <libbsvg/SVGView.h>

BBitmap *
render_SVG( const char * path, float width, float height = -1.0 )
{
	if ( height < 0 )
		height = width;
	
	BBitmap * bmp = new BBitmap(
		BRect(0,0,width-1, height-1),
		B_RGB32,
		true
	);
	
	BSVGView * view = new BSVGView(
		BRect(0,0,width-1, height-1),
		"SVG render view",
		B_FOLLOW_NONE
	);
	
	view->SetScaleToFit( true );
	view->SetFitContent( true );
	view->SetViewColor( B_TRANSPARENT_COLOR );
	view->SetSampleSize(3);
	
	bmp->AddChild( view );
	
	bmp->Lock();

	if ( view->LoadFromFile( path ) != B_OK )
	{
		printf("Loading SVG failed\n");
		delete view;
		delete bmp;
		
		return NULL;
	}
	
	view->FrameResized( view->Bounds().Width(), view->Bounds().Height() );
	view->Draw( view->Bounds() );
	view->Sync();
	bmp->Unlock();
	
	bmp->RemoveChild( view );
	
	delete view;
	
	return bmp;
}
*/


ChatWindow::ChatWindow(entry_ref & ref)
:	BWindow( 
		BRect(100,100,400,300), 
		"unknown contact - unknown status", 
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AVOID_FOCUS
	),
	fEntry(ref),
	fMan( new IM::Manager(BMessenger(this))),
	fChangedNotActivated(false),
	fStatusBar(NULL),
	fSendButton(NULL),
	fProtocolHack(NULL)
{

	AddShortcut(B_LEFT_ARROW, B_SHIFT_KEY, new BMessage(PREV_WIN));
	AddShortcut(B_RIGHT_ARROW, B_SHIFT_KEY, new BMessage(NEXT_WIN));

	bool command, sendButton;
	int32 iconBarSize;
	
	BMessage chatSettings;
	im_load_client_settings("im_client", &chatSettings);
	
	if ( chatSettings.FindBool("command_sends", &command) != B_OK )
		command = true;
	if ( chatSettings.FindBool("show_send_button", &sendButton) != B_OK )
		sendButton = true;
	if ( chatSettings.FindInt32("icon_size", &iconBarSize) != B_OK )
		iconBarSize = kLargeIcon;
	if ( iconBarSize <= 0 )
		iconBarSize = kLargeIcon;
	if (chatSettings.FindString("people_handler", &fPeopleHandler) != B_OK) {
		fPeopleHandler = kDefaultPeopleHandler;
	};
	
	// Set window size limits
	SetSizeLimits(
		220, 8000, // width,
		150, 8000  // height
	);
	
	// get the size of various things
	font_height height;
	be_plain_font->GetHeight(&height);
	fFontHeight = height.ascent + height.descent + height.leading;
	
	// default window size
	BRect windowRect(100, 100, 400, 300);
	BPoint inputDivider(0, 150);
	
	// load window size if possible
	if (LoadSettings() == B_OK) {
		bool was_ok = true;
		
		if (fWindowSettings.FindRect("windowrect", &windowRect) != B_OK) {
			was_ok = false;
		}
		if (fWindowSettings.FindPoint("inputdivider", &inputDivider) != B_OK) {
			was_ok = false;
		}
		
		if ( !was_ok )
		{
			windowRect = BRect(100, 100, 400, 300);
			inputDivider = BPoint(0, 200);
		}
	}
	
	// sanity check for divider location
	if ( inputDivider.y > windowRect.Height() - 50 ) {
		LOG("im_client", liLow, "Insane divider, fixed.");
		inputDivider.y = windowRect.Height() - 50;
	};
	
	// set size and position
	MoveTo(windowRect.left, windowRect.top);
	ResizeTo(windowRect.Width(), windowRect.Height());


	BPath settings;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settings, true);
	settings.Append("im_kit/im_client/toolbar");

	// create views
	BRect textRect = Bounds();
	BRect inputRect = Bounds();
	BRect dockRect = Bounds();

	dockRect.bottom = iconBarSize + kDockPadding;
	fDock = new IconBar(dockRect, settings.Path(), &gBubbles, kPadding, ref);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fDock->SetViewUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fDock->SetLowUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fDock->SetHighUIColor(B_UI_PANEL_TEXT_COLOR);
#else
	fDock->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	fDock->SetLowColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	fDock->SetHighColor(0, 0, 0, 0);
#endif
	AddChild(fDock);

	textRect.top = fDock->Bounds().bottom+1;
	textRect.InsetBy(2,2);
	textRect.bottom = inputDivider.y;
	textRect.right -= B_V_SCROLL_BAR_WIDTH;
	
	float sendButtonWidth = sendButton ? 50 : 0;
	
	inputRect.InsetBy(2.0, 2.0);
	inputRect.top = inputDivider.y + 7;
	inputRect.right -= B_V_SCROLL_BAR_WIDTH + sendButtonWidth;
	inputRect.bottom -= fFontHeight + (kPadding * 4);
	
	BRect inputTextRect = inputRect;
	inputTextRect.OffsetTo(kPadding, kPadding);
	inputTextRect.InsetBy(kPadding * 2, kPadding * 2);
	
	fInput = new BTextView(inputRect, "input", inputTextRect, B_FOLLOW_ALL,
		B_WILL_DRAW);
	
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fInput->SetViewUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fInput->SetLowUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fInput->SetHighUIColor(B_UI_DOCUMENT_TEXT_COLOR);
#else
	fInput->SetViewColor(245, 245, 245, 0);
	fInput->SetLowColor(245, 245, 245, 0);
	fInput->SetHighColor(0, 0, 0, 0);
#endif

	fInputScroll = new BScrollView(
		"input_scroller", fInput,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, 0,
		false,
		true
	);

	AddChild(fInputScroll);	
	
	fInput->SetWordWrap(true);
	fInput->SetStylable(false);
	fInput->MakeSelectable(true);
	
	if (sendButton) {
		BRect sendRect = fInputScroll->Frame();
		sendRect.left = sendRect.right+1;
		sendRect.right = Bounds().right;
		
		fSendButton = new BButton(sendRect, "sendButton", _T("Send"),
			new BMessage(SEND_MESSAGE), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	
		AddChild(fSendButton);
	}
	
	BRect statusRect = Bounds();
	statusRect.top = inputRect.bottom + kPadding;
	fStatusBar = new StatusBar(statusRect);
	
	AddChild(fStatusBar);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fStatusBar->SetViewUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fStatusBar->SetLowUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fStatusBar->SetHighUIColor(B_UI_PANEL_TEXT_COLOR);
#else
	fStatusBar->SetViewColor(245, 245, 245, 0);
	fStatusBar->SetLowColor(245, 245, 245, 0);
	fStatusBar->SetHighColor(0, 0, 0, 0);
#endif
		
	BPopUpMenu *pop = new BPopUpMenu("Protocols", true, true);
	fProtocolMenu = new BMenuField(
		BRect(kPadding, kPadding, Bounds().bottom - kPadding, 100),
		"Field", NULL, pop);
	fStatusBar->AddItem(fProtocolMenu);

	entry_ref keyRef;
	get_ref_for_path("/boot/preferences/Keyboard", &keyRef);
	fTypingView = new IconView(keyRef, fStatusBar->Frame().Height() - kPadding, true);
	fTypingView->EnableDrawing(false);

	entry_ref mobRef;
	get_ref_for_path("/boot/home/config/settings/im_kit/icons/MobileUser", &mobRef);
	fMobileView = new IconView(mobRef, fStatusBar->Frame().Height() - kPadding, true);
	fMobileView->EnableDrawing(false);

	fStatusBar->AddItem(fTypingView);
	fStatusBar->AddItem(fMobileView);
	fStatusBar->PositionViews();
	fTypingView->MoveBy(0, kPadding);
	fMobileView->MoveBy(0, kPadding);

	BuildProtocolMenu();
	BMenuItem *first = pop->ItemAt(0);
	if (first) first->SetMarked(true);

	BRect resizeRect = Bounds();
	resizeRect.top = inputDivider.y + 1;
	resizeRect.bottom = inputDivider.y + 4;
	
	fResize = new ResizeView(fInputScroll, resizeRect, "resizer",
		B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT);
	AddChild(fResize);
	
	Theme::TimestampFore = C_TIMESTAMP_DUMMY;
	Theme::TimestampBack = C_TIMESTAMP_DUMMY;
	Theme::TimespaceFore = MAX_COLORS;
	Theme::TimespaceBack = MAX_COLORS;
	Theme::TimespaceFont = MAX_FONTS;
	Theme::TimestampFont = F_TIMESTAMP_DUMMY;
	Theme::NormalFore = C_TEXT;
	Theme::NormalBack = C_TEXT;
	Theme::NormalFont = F_TEXT;
	Theme::SelectionBack = C_SELECTION;
	
	fTheme = new Theme("ChatWindow", MAX_COLORS + 1, MAX_COLORS + 1, MAX_FONTS + 1);

	fTheme->WriteLock();
	fTheme->SetForeground(C_URL, 5, 5, 150);
	fTheme->SetBackground(C_URL, 255, 255, 255);
	fTheme->SetFont(C_URL, be_plain_font);
	
	fTheme->SetForeground(C_TIMESTAMP, 130, 130, 130);
	fTheme->SetBackground(C_TIMESTAMP, 255, 255, 255);
	fTheme->SetFont(F_TIMESTAMP, be_plain_font);

	fTheme->SetForeground(C_TEXT, 0, 0, 0);
	fTheme->SetBackground(C_TEXT, 255, 255, 255);
	fTheme->SetFont(F_TEXT, be_plain_font);
	
	fTheme->SetForeground(C_ACTION, 0, 0, 0);
	fTheme->SetBackground(C_ACTION, 255, 255, 255);
	fTheme->SetFont(F_ACTION, be_plain_font);
	
	fTheme->SetForeground(C_SELECTION, 255, 255, 255);
	fTheme->SetBackground(C_SELECTION, 0, 0, 0);

	fTheme->SetForeground(C_OWNNICK, 0, 0, 255);
	fTheme->SetBackground(C_OWNNICK, 255, 255, 255);
	
	fTheme->SetForeground(C_OTHERNICK, 255, 0, 0);
	fTheme->SetBackground(C_OTHERNICK, 255, 255, 255);

	fTheme->WriteUnlock();
	
	IM::Contact con(&fEntry);
	char id[256];
	con.ConnectionAt(0, id);

	fText = ((ChatApp *)be_app)->GetRunView(id);
	if (fText == NULL) {
		fText = new RunView(textRect, "text", fTheme, B_FOLLOW_ALL, B_WILL_DRAW);
	};
	
	fText->SetTimeStampFormat(NULL);

#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fText->SetViewUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fText->SetLowUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fText->SetHighUIColor(B_UI_DOCUMENT_TEXT_COLOR);
#else
	fText->SetViewColor(245, 245, 245, 0);
	fText->SetLowColor(245, 245, 245, 0);
	fText->SetHighColor(0, 0, 0, 0);
#endif

	fTextScroll = new BScrollView(
		"scroller", fText,
		B_FOLLOW_ALL, 0,
		false, // horiz
		true // vert
	);
	AddChild(fTextScroll);
	fTextScroll->MoveTo(0,fDock->Bounds().bottom+1);
	
	fText->Show();
	fText->ScrollToBottom();

	fInput->MakeFocus();
	
	// add input filter that generates "user typing" messages and routes copy-commands
	fFilter = new InputFilter(fInput, new BMessage(SEND_MESSAGE), command, fText,
		kTypingSendRate);
	fInput->AddFilter((BMessageFilter *)fFilter);
		
	// monitor node so we get updates to status etc
	BEntry entry(&ref);
	node_ref node;
	
	entry.GetNodeRef(&node);
	watch_node( &node, B_WATCH_ALL, BMessenger(this) );
	
	// get contact info
	reloadContact();

	// set up timer for clearing typing view
	fTypingTimer = NULL;
	fTypingTimerSelf = NULL;
	
	// this message runner needed to fix a BMenuField bug.
	BMessage protoHack(PROTOCOL_SELECTED2);
	fProtocolHack = new BMessageRunner( BMessenger(this), &protoHack, 1000, 1 );
	
	BMessage contactInfo;
	BMessage getContactInfo(IM::MESSAGE);
	getContactInfo.AddInt32("im_what", IM::GET_CONTACT_INFO);
	getContactInfo.AddRef("contact", &fEntry);
	fMan->SendMessage(&getContactInfo, &contactInfo);
	
	bool mobile;
	if (contactInfo.FindBool("mobileuser", &mobile) == B_OK) {
		fMobileView->EnableDrawing(mobile);
		gBubbles.SetHelp(fMobileView, (char *)_T(kMobileDevice));
	} else {
		fMobileView->EnableDrawing(false);
		gBubbles.SetHelp(fMobileView, NULL);
	};
	
	contactInfo.PrintToStream();
}

ChatWindow::~ChatWindow()
{
	stopNotify();
	SaveSettings();
	
	stop_watching( BMessenger(this) );

	if (fInput) {
		if (fInput->RemoveFilter(fFilter)) delete fFilter;
		fInput->RemoveSelf();
		
		delete fInput;
	};

	if (fResize) {
		fResize->RemoveSelf();
		delete fResize;
	};

	if (fDock) {
		for ( int i=0; i<fDock->CountChildren(); i++ )
		{
			gBubbles.SetHelp( fDock->ChildAt(i), NULL );
		}
		
		fDock->RemoveSelf();
		delete fDock;
	};

	if (fStatusBar) fStatusBar->RemoveSelf();
	delete fStatusBar;

	if ( fProtocolHack )
		delete fProtocolHack;
	
	fMan->Lock();
	fMan->Quit();
}

//#pragma mark -

bool
ChatWindow::QuitRequested()
{
	if (fTextScroll != NULL) {
		fTextScroll->RemoveSelf();
//		Deleting the fTextScroll here causes a crash when you re open the window.
//		This makes Baby Mikey cry
//		delete fTextScroll;
	};

	fText->RemoveSelf();

	IM::Contact con(&fEntry);
	char id[256];
	con.ConnectionAt(0, id);
	((ChatApp *)be_app)->StoreRunView(id, fText);
	((ChatApp *)be_app)->chat_windows.RemoveItem(this);

	return true;
}

status_t
ChatWindow::SaveSettings(void) {
	if (fWindowSettings.ReplaceRect("windowrect", Frame()) != B_OK) {
		fWindowSettings.AddRect("windowrect", Frame());
	};
	
	BRect resizeRect = fResize->Frame();
	BPoint resize(0, resizeRect.top-1);
	
	if (fWindowSettings.ReplacePoint("inputdivider", resize) != B_OK) {
		fWindowSettings.AddPoint("inputdivider", resize);
	};

	ssize_t size = fWindowSettings.FlattenedSize();
	char *buffer = (char *)calloc(size, sizeof(char));

	status_t res = B_OK;
	
	if (fWindowSettings.Flatten(buffer, size) != B_OK) {
		LOG("im_client", liHigh, "Could not flatten window settings");
		res = B_ERROR;
	} else {
		LOG("im_client", liLow, "Window settings flattened");
		BNode peopleNode(&fEntry);
		
		if (peopleNode.WriteAttr("IM:ChatSettings", B_MESSAGE_TYPE, 0, buffer, 
			(size_t)size) == size) {
			LOG("im_client", liLow, "Window Settings saved to disk");
		} else {
			LOG("im_client", liHigh, "Window settings could not be written to disk");
			res = B_ERROR;
		};	
	};
	
	free(buffer);
	
	return res;
}

status_t
ChatWindow::LoadSettings(void) {
	// read status
	
	BNode peopleNode(&fEntry);
	attr_info info;
	
	if (peopleNode.GetAttrInfo("IM:ChatSettings", &info) == B_OK) {	
		char *buffer = (char *)calloc(info.size, sizeof(char));
		
		if (peopleNode.ReadAttr("IM:ChatSettings", B_MESSAGE_TYPE, 0,
			buffer, (size_t)info.size) == info.size) {
			
			if (fWindowSettings.Unflatten(buffer) == B_OK) {
				return B_OK;
			} else {
				LOG("im_client", liLow, "Could not unflatten settings messsage");
				return B_ERROR;
			};

			free(buffer);
		} else {
			LOG("im_client", liLow, "Could not read chat attribute");
			return B_ERROR;
		};
	} else {
		LOG("im_client", liLow, "Could not load chat settings");
		return B_ERROR;
	};
	
	return B_OK;
}

void
ChatWindow::MessageReceived( BMessage * msg )
{
	switch ( msg->what )
	{
		case PREV_WIN: 
		case NEXT_WIN: {
			int32 add = 1;
			int32 windows = be_app->CountWindows();
			int32 next = -10;
			std::vector<BWindow *> chatWindows;
			
			if (msg->what == PREV_WIN) add = -1;
			
			for (int32 i = 0; i < windows; i++) {
				BWindow *window = be_app->WindowAt(i);
				if (dynamic_cast<ChatWindow *>(window) != NULL) {
					chatWindows.push_back(window);
				};
				if (window == this) next = (chatWindows.size() - 1) + add;
			};
			
			if (next < -1) return;			
			if (next < 0) next = chatWindows.size() - 1;
			if (next > (int32)(chatWindows.size() - 1)) next = 0;
			
			BWindow *window = chatWindows[next];
			if (window) window->Activate(true);
		};
		
		case IM::CONTACT_INFO: {
			msg->PrintToStream();
		} break;
	
		case IM::SETTINGS_UPDATED: {
			if (msg->FindString("people_handler", &fPeopleHandler) != B_OK) {
				fPeopleHandler = kDefaultPeopleHandler;
			};
			
			RebuildDisplay();
		} break;
		case IM::USER_STOPPED_TYPING: {
			BMessage im_msg(IM::MESSAGE);
			im_msg.AddInt32("im_what",IM::USER_STOPPED_TYPING);
			im_msg.AddRef("contact",&fEntry);
			fMan->SendMessage(&im_msg);
			
			stopSelfTypingTimer();
		} break;
		case IM::USER_STARTED_TYPING: {
			BMessage im_msg(IM::MESSAGE);
			im_msg.AddInt32("im_what", IM::USER_STARTED_TYPING);
			im_msg.AddRef("contact", &fEntry);
			fMan->SendMessage(&im_msg);
			
			startSelfTypingTimer();
		} break;
		case IM::DESKBAR_ICON_CLICKED:
		{ // deskbar icon clicked, move to current workspace and activate
			SetWorkspaces( 1 << current_workspace() );
			Activate();
		}	break;
		
		case IM::ERROR:
		case IM::MESSAGE:
		{
			entry_ref contact;
			
			if ( msg->FindRef("contact",&contact) != B_OK )
				return;
				
			if ( contact != fEntry )
				// message not for us, skip it.
				return;
			
			int32 im_what=IM::ERROR;
			
			if ( msg->FindInt32("im_what",&im_what) != B_OK )
				im_what = IM::ERROR;
						
			char timestr[10];
			time_t now = time(NULL);
			strftime(timestr, sizeof(timestr),"[%H:%M]: ", localtime(&now) );
				
			switch ( im_what )
			{
				case IM::STATUS_CHANGED:
				{
					// This means we're rebuilding menus we don't really need to rebuild..
					BuildProtocolMenu();
				}	break;
				
				case IM::MESSAGE_SENT:
				{
					fText->Append(timestr, C_TIMESTAMP, C_TIMESTAMP, F_TIMESTAMP);

					BString message;
					msg->FindString("message", &message);
					if (message.Compare("/me ", 4) == 0) {
						fText->Append(_T("* You "), C_ACTION, C_ACTION, F_ACTION);
						message.Remove(0, 4);
						fText->Append(message.String(), C_ACTION, C_ACTION, F_ACTION);
					} else {
						fText->Append(_T("You say: "), C_OWNNICK, C_OWNNICK, F_TEXT);
						fText->Append(msg->FindString("message"), C_TEXT, C_TEXT, F_TEXT);
					}
					fText->Append("\n", C_TEXT, C_TEXT, F_TEXT);
					fText->ScrollToSelection();
				}	break;
				
				case IM::ERROR:
				{
					BMessage error;
					msg->FindMessage("message", &error);
					
					int32 error_what = -1;
					
					error.FindInt32("im_what", &error_what );
					
					if ( error_what != IM::USER_STARTED_TYPING && 
						 error_what != IM::USER_STOPPED_TYPING )
					{ // ignore messages du to typing
						fText->Append(timestr, C_TIMESTAMP, C_TIMESTAMP, F_TIMESTAMP);
						fText->Append("Error: ", C_TEXT, C_TEXT, F_TEXT);
						fText->Append(msg->FindString("error"), C_TEXT, C_TEXT, F_TEXT);
						fText->Append("\n", C_TEXT, C_TEXT, F_TEXT);
					
						if (!IsActive()) startNotify();
					}
				}	break;
				
				case IM::MESSAGE_RECEIVED:
				{
					if ( msg->FindString("message") == NULL )
					{ // no message to display, probably opened by user
						return;
					}
					
					fText->Append(timestr, C_TIMESTAMP, C_TIMESTAMP, F_TIMESTAMP);
					
					BString protocol = msg->FindString("protocol");
					BString message = msg->FindString("message");
					
					if (protocol.Length() > 0) {
						protocol.Prepend(" (");
						protocol << ") ";
					} else {
						protocol = " ";
					};
					
					if (message.Compare("/me ", 4) == 0) {
						fText->Append("* ", C_ACTION, C_ACTION, F_ACTION);
						fText->Append(fName, C_ACTION, C_ACTION, F_ACTION);
						fText->Append(protocol.String(), C_ACTION, C_ACTION, F_ACTION);
						message.Remove(0, 4);
						fText->Append(message.String(), C_ACTION, C_ACTION, F_ACTION);
					} else {
						fText->Append(fName, C_OTHERNICK, C_OTHERNICK, F_TEXT);
						fText->Append(protocol.String(), C_OTHERNICK, C_OTHERNICK, F_TEXT);
						fText->Append(": ", C_OTHERNICK, C_OTHERNICK, F_TEXT);
						fText->Append(msg->FindString("message"), C_TEXT, C_TEXT, F_TEXT);
					}
					fText->Append("\n", C_TEXT, C_TEXT, F_TEXT);
					fText->ScrollToSelection();

					if (!IsActive()) startNotify();
					
					stopTypingTimer();
				}	break;
				
				case IM::CONTACT_STARTED_TYPING: {	
					startTypingTimer();
				} break;
				
				case IM::CONTACT_STOPPED_TYPING: {
					stopTypingTimer();
				} break;
				
				case IM::CONTACT_INFO: {
					bool mobile;
					if (msg->FindBool("mobileuser", &mobile) == B_OK) {
						fMobileView->EnableDrawing(mobile);
					};
					if (mobile) {
						gBubbles.SetHelp(fMobileView, (char *)_T(kMobileDevice));
					} else {
						gBubbles.SetHelp(fMobileView, NULL);
					};
					fMobileView->Invalidate();
				} break;
			}
			
			fText->ScrollToSelection();
			
		}	break;
		
		case SEND_MESSAGE:
		{
			if (fInput->TextLength() == 0) return;
			BMessage im_msg(IM::MESSAGE);
			im_msg.AddInt32("im_what",IM::SEND_MESSAGE);
			im_msg.AddRef("contact",&fEntry);
			im_msg.AddString("message", fInput->Text() );
			
			BMenu *menu = fProtocolMenu->Menu();
			if (menu) {
				IconMenuItem *item = dynamic_cast<IconMenuItem*>(menu->FindMarked());
				if ( item )
				{
					BString connection = item->Extra();
					if (connection.Length() > 0) 
					{
						IM::Connection conn(connection.String());
						
						im_msg.AddString("protocol", conn.Protocol());
						im_msg.AddString("id", conn.ID());
					}
				}	
			};
			
			if ( fMan->SendMessage(&im_msg) == B_OK ) {
				fInput->SetText("");
			} else {
				LOG("im_client", liHigh, "Error sending message to im_server");

				fText->Append(_T("Error: im_server not running, can't send message\n"), C_TEXT, C_TEXT, F_TEXT);
					
				fText->ScrollToSelection();
			};
		}	break;
		
		case B_NODE_MONITOR:
		{
			int32 opcode=0;
			
			if ( msg->FindInt32("opcode",&opcode) != B_OK )
				return;
			
			switch ( opcode )
			{
				case B_ENTRY_REMOVED: {
					// oops. should we close down this window now?
					// Nah, we'll just disable everything.
					fInput->MakeEditable(false);
					fInput->SetViewColor( 198,198,198 );
					fInput->Invalidate();
					
					BString title( Title() );
					title += " - DELETED!";
					SetTitle( title.String() );
				}	break;
				case B_ENTRY_MOVED:
				{
					entry_ref ref;
					
					msg->FindInt32("device", &ref.device);
					msg->FindInt64("to directory", &ref.directory);
					ref.set_name( msg->FindString("name") );
					
					fEntry = ref;
					
					BEntry entry(&fEntry);
					if ( !entry.Exists() )
					{
						LOG("im_client", liHigh, "Entry moved: New entry invalid");
					}
				}	break;
				case B_STAT_CHANGED:
				case B_ATTR_CHANGED:
					reloadContact();
					BuildProtocolMenu();
					break;
			}
		}	break;
		
		case kResizeMessage: {
			BView *view = NULL;
			msg->FindPointer("view", reinterpret_cast<void**>(&view));
			if (dynamic_cast<BScrollView *>(view)) {
				BPoint point;
				msg->FindPoint("loc", &point);
				
				fResize->MoveTo(fResize->Frame().left, point.y);
				
				fTextScroll->ResizeTo(fTextScroll->Frame().Width(), point.y - 1 - fDock->Frame().Height() - 1);
				
				fInputScroll->MoveTo(fInputScroll->Frame().left, point.y + 3);
				fInputScroll->ResizeTo( 
					fInputScroll->Bounds().Width(),
					fStatusBar->Frame().top - fInputScroll->Frame().top
				);
				fInput->SetTextRect(fInput->Bounds());
				fInput->ScrollToSelection();
				
				if ( fSendButton )
				{
					fSendButton->MoveTo(fSendButton->Frame().left, point.y + 3);
					fSendButton->ResizeTo( 
						fSendButton->Bounds().Width(),
						fStatusBar->Frame().top - fSendButton->Frame().top
					);
				}
			};
		} break;
		
		case B_MOUSE_WHEEL_CHANGED: {
			fText->MessageReceived(msg);
		} break;
		
		case B_COPY: {
			int32 start = 0;
			int32 end = 0;
			
			fInput->GetSelection(&start, &end);
		} break;
		
		case B_SIMPLE_DATA: {
			entry_ref ref;
			BNode node;
			
			for (int i = 0; msg->FindRef("refs", i, &ref) == B_OK; i++) {
				node = BNode(&ref);
				
				char *type = ReadAttribute(node, "BEOS:TYPE");
				if (strcmp(type, "application/x-person") == 0) {
					char *name = ReadAttribute(node, "META:name");
					char *nickname = ReadAttribute(node, "META:nickname");
					char connection[100];
					IM::Contact con(ref);
					con.ConnectionAt(0, connection);

					if (fInput->TextLength() > 0) fInput->Insert("\n");
					fInput->Insert(name);
					fInput->Insert(" (");
					fInput->Insert(nickname);
					fInput->Insert("): ");
					fInput->Insert(connection);

					free(name);
					free(nickname);
				};
				free(type);
			};
			fInput->ScrollToOffset(fInput->TextLength());
		} break;
		
		case CLEAR_TYPING:
			stopTypingTimer();
			break;
		
		case PROTOCOL_SELECTED: {
			// a protocol has been selected. Since BMenuField doesn't resize until later,
			// we have to wait 1000us before actually responding to the change, see below
			if ( fProtocolHack )
				delete fProtocolHack;
			BMessage protoHack(PROTOCOL_SELECTED2);
			fProtocolHack = new BMessageRunner( BMessenger(this), &protoHack, 1000, 1 );
		}	break;
		
		case PROTOCOL_SELECTED2: {
			// do what should be done on protocol change
			fStatusBar->PositionViews();
		} break;
		
		default:
			BWindow::MessageReceived(msg);
	}
}

void
ChatWindow::FrameResized( float /*w*/, float /*h*/ )
{
	fText->ScrollToSelection();
	
	fInput->SetTextRect(fInput->Bounds());
	fInput->ScrollToSelection();
}

void
ChatWindow::WindowActivated(bool active)
{
	if (active) 
		stopNotify();
	
	BWindow::WindowActivated(active);
}

bool
ChatWindow::handlesRef( entry_ref & ref )
{
	return ( strcmp(BPath(&fEntry).Path(),BPath(&ref).Path()) == 0 );
}

void
ChatWindow::reloadContact()
{
	IM::Contact c(&fEntry);
	
	char status[512];
	char name[512];
	char nick[512];
	
	int32 num_read;
	
	// read name
	if ( c.GetName(name,sizeof(name)) != B_OK )
		strcpy(name,_T("Unknown name"));
	
	if ( c.GetNickname(nick,sizeof(nick)) != B_OK )
		strcpy(nick,_T("no nick"));
	
	sprintf(fName,"%s (%s)", name, nick );
	
	BNode node(&fEntry);
	
	// read status
	num_read = node.ReadAttr(
		"IM:status", B_STRING_TYPE, 0,
		status, sizeof(status)-1
	);
	
	if ( num_read <= 0 )
		strcpy(status,_T("Unknown status"));
	else
		status[num_read] = 0;
	
	// rename window
	sprintf(fTitleCache,"%s - %s", fName, _T(status));
	
	if ( !fChangedNotActivated )
	{
		SetTitle(fTitleCache);
	} else
	{
		char str[512];
		sprintf(str, "√ %s", fTitleCache);
		SetTitle(str);
	}
}

void
ChatWindow::startNotify()
{
	if ( fChangedNotActivated )
		return;
	
	fChangedNotActivated = true;
	char str[512];
	sprintf(str, "√ %s", fTitleCache);
	SetTitle(str);
	
	((ChatApp*)be_app)->Flash( BMessenger(this) );
	
//	if ( (Workspaces() & (1 << current_workspace())) == 0) // beep if on another workspace
	if ( !IsActive() ) // beep if not active
	{
		system_beep(kImNewMessageSound);
	}
}

void
ChatWindow::stopNotify()
{
	if ( !fChangedNotActivated )
		return;
	
	fChangedNotActivated = false;
	SetTitle(fTitleCache);
	((ChatApp*)be_app)->NoFlash( BMessenger(this) );
}


//#pragma mark -

void ChatWindow::BuildProtocolMenu(void) {
	BMessage getStatus(IM::GET_CONTACT_STATUS);
	getStatus.AddRef("contact", &fEntry);
	BMessage statusMsg;

	BMenu *menu = fProtocolMenu->Menu();
	if (menu == NULL) {
		LOG("im_client", liHigh, "BuildProtocolMenu(): fProtocolMenu is NULL.");
		return;
	}

//	You have to do this twice... buggered if I know why...
	for (int32 i = 0; i < menu->CountItems(); i++) delete menu->RemoveItem(0L);
	for (int32 i = 0; i < menu->CountItems(); i++) delete menu->RemoveItem(0L);

	menu->AddItem(new IconMenuItem(NULL, _T("Any Protocol"), NULL,
		new BMessage(PROTOCOL_SELECTED)));
	
	if (fMan->SendMessage(&getStatus, &statusMsg) != B_OK) {
		LOG("im_client", liHigh, "Failed to get contact statues");
		return;
	};
	
	BPath iconDir;
	find_directory(B_USER_ADDONS_DIRECTORY, &iconDir, true);
	iconDir.Append("im_kit/protocols");
			
	menu->AddSeparatorItem();
	
	for (int32 i = 0; statusMsg.FindString("connection", i); i++) {
		BString status = statusMsg.FindString("status", i);
		IM::Connection connection( statusMsg.FindString("connection", i) );

		BString iconPath = iconDir.Path();
		iconPath << "/" << connection.Protocol();
			
		BBitmap *icon = ReadNodeIcon(iconPath.String(), kSmallIcon, true);
		BString label = connection.String();
		label << " (" << _T(status.String()) << ")";
			
		menu->AddItem(
			new IconMenuItem(
				icon, 
				label.String(), 
				connection.String(), 
				new BMessage(PROTOCOL_SELECTED)
			)
		);
	};
	
	// TODO: Is this call needed or not?
	//menu->SetFont(be_plain_font);
	
	fStatusBar->PositionViews();
};

void ChatWindow::startTypingTimer(void) {
	if (fTypingTimer) delete fTypingTimer;
	
	BMessage clearTyping(CLEAR_TYPING);
	fTypingTimer = new BMessageRunner(BMessenger(this), &clearTyping,
		kTypingStoppedRate, 1);
		
	if (fTypingTimer->InitCheck() != B_OK)
		LOG("im_client", liHigh, "InitCheck fail on typing timer");

	fTypingView->EnableDrawing(true);
	fTypingView->Invalidate();
	gBubbles.SetHelp(fTypingView, (char *)_T("User is typing"));
};

void
ChatWindow::stopTypingTimer(void)
{
	if (fTypingTimer) delete fTypingTimer;
	fTypingTimer = NULL;

	fTypingView->EnableDrawing(false);
	fTypingView->Invalidate();
	gBubbles.SetHelp(fTypingView, NULL);
}

void ChatWindow::startSelfTypingTimer(void) {
	if (fTypingTimerSelf) delete fTypingTimerSelf;
	
	fTypingTimerSelf = new BMessageRunner(BMessenger(this),
		new BMessage(IM::USER_STOPPED_TYPING), kTypingStoppedRate, 1);
	if (fTypingTimerSelf->InitCheck() != B_OK) {
		LOG("im_client", liHigh, "Initcheck failed on self-typing timer");
	};
};

void ChatWindow::stopSelfTypingTimer(void) {
	if (fTypingTimerSelf) delete fTypingTimerSelf;
	fTypingTimerSelf = NULL;
};

void ChatWindow::RebuildDisplay(void) {
#if 0

	BMessage chatSettings;
	int32 iconBarSize;
	
	im_load_client_settings("im_client", &chatSettings);
	
//	if ( chatSettings.FindBool("command_sends", &command) != B_OK )
//		command = true;
//	if ( chatSettings.FindBool("show_send_button", &sendButton) != B_OK )
//		sendButton = true;
	if ( chatSettings.FindInt32("icon_size", &iconBarSize) != B_OK )
		iconBarSize = kLargeIcon;
	if (iconBarSize <= 0) iconBarSize = kLargeIcon;
	
	printf("Resizing to %i\n", iconBarSize);
	fDock->ResizeTo(Bounds().Width(), iconBarSize + kDockPadding);
#endif
};

BBitmap *ChatWindow::IconForHandler(const char *type, int32 size) {
	entry_ref handlerRef;
	
	if (be_roster->FindApp(type, &handlerRef) != B_OK) {
		LOG("im_client", liMedium, "Failed to get icon for %s", type);
//		this isn't what we should be doing, but it might be better than nothing.
		handlerRef = fEntry;
	};
	
	BPath handlerPath(&handlerRef);
	return ReadNodeIcon(handlerPath.Path(), size);
};

ImageButton *ChatWindow::MakeButton(BBitmap *icon, const char *help,
	BMessage *msg, BRect rect) {
	
	ImageButton *button = new ImageButton(rect, help, msg, B_FOLLOW_NONE,
		B_WILL_DRAW, icon, NULL);
	gBubbles.SetHelp(button, (char *)_T(help));
	
	return button;	
};
