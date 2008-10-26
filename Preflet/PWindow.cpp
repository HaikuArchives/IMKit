#include "PWindow.h"

#include <libim/Helpers.h>
#include <Entry.h>
#include <Roster.h>
#include <ScrollView.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

const float kControlOffset = 5.0;
const float kEdgeOffset = 5.0;
const float kDividerWidth = 100;

BubbleHelper gHelper;

PWindow::PWindow(void)
	: BWindow(BRect(5, 25, 480, 285), "Instant Messaging", B_TITLED_WINDOW,
	 B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS) {
#ifdef ZETA
	app_info ai;
	be_app->GetAppInfo( &ai );
	BPath path;
	BEntry entry( &ai.ref, true );
	entry.GetPath( &path );
	path.GetParent( &path );
	path.Append( "Language/Dictionaries/InstantMessaging" );
	BString path_string;
	
	if( path.InitCheck() != B_OK )
		path_string.SetTo( "Language/Dictionaries/InstantMessaging" );
	else
		path_string.SetTo( path.Path() );
	
	be_locale.LoadLanguageFile( path_string.String() );
#endif
	
	fManager = new IM::Manager(BMessenger(this));
	
	fCurrentIndex = 0;
	fCurrentView = NULL;
	fBox = NULL;
	fView = NULL;
	fSave = NULL;
	fRevert = NULL;
	fListView = NULL;
	fBox = NULL;

	BRect frame = Bounds();

	fView = new BView(frame, "PrefView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	AddChild(fView);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(0, 0, 0, 0);
#endif

	frame.left = kEdgeOffset;
	frame.top = kEdgeOffset;
	frame.bottom = Bounds().bottom - kEdgeOffset;
	frame.right = 120;
	fListView = new BOutlineListView(frame, "LISTVIEW", B_SINGLE_SELECTION_LIST);

	font_height fontHeight;
	be_bold_font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.descent + fontHeight.leading + fontHeight.ascent;

	fBox = new BBox(BRect(fListView->Bounds().right + (kEdgeOffset * 3) + 
		B_V_SCROLL_BAR_WIDTH, kEdgeOffset, 	fView->Bounds().right - kEdgeOffset,
		fView->Bounds().bottom - ((fFontHeight * 2) + kEdgeOffset)), "BOX",
		B_FOLLOW_ALL_SIDES);
	fBox->SetLabel("IM Server");
	
	fView->AddChild(fBox);

	frame = fBox->Bounds();
	frame.InsetBy(kEdgeOffset, kEdgeOffset);
	frame.top += fFontHeight;
	frame.right -= B_V_SCROLL_BAR_WIDTH + 2;
	// PROTOCOLS

	IconTextItem *protoItem = new IconTextItem(_T("Protocols"), NULL);
	fListView->AddItem(protoItem);
	
	BMessage protocols;
	im_get_protocol_list(&protocols);
	
	if (protocols.FindString("protocol")) {
		const char *protocol = NULL;
//		FIX ME: Find the location of the im_server programmatically (By app signature?)
		for (int16 i = 0; protocols.FindString("protocol", i, &protocol) == B_OK; i++) {
			entry_ref ref;
			//protocols.FindRef("ref", i, &ref);	
			
//			XXX Fix Me: Change to use find_directory()
			BString protoPath = "/boot/home/config/add-ons/im_kit/protocols/";
			protoPath << protocol;
			
			BMessage protocol_settings;
			BMessage protocol_template;
			
			im_load_protocol_settings( protocol, &protocol_settings );
			im_load_protocol_template( protocol, &protocol_template );
			
			BBitmap *icon = ReadNodeIcon(protoPath.String(), kSmallIcon, true);
			IconTextItem *item = new IconTextItem(protocol, icon);
			fListView->AddUnder(item, protoItem);
			
			protocol_template.AddString("protocol", protocol); // for identification when saving
			
			pair <BMessage, BMessage> p(protocol_settings, protocol_template);
			fAddOns[protocol] = p; //pair<BMessage, BMessage>(settings, tmplate);
			
			BView *view = new BView(frame, protocol, B_FOLLOW_NONE,
				B_WILL_DRAW);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
			view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
			view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetHighColor(0, 0, 0, 0);
#endif
			
			float settings_height = BuildGUI(protocol_template, protocol_settings, view);
			
			BScrollView * scroller = new BScrollView(
				"scroller", view, B_FOLLOW_ALL,
				0, false, true
			);
			view->ResizeTo( view->Bounds().Width(), settings_height );
			
			float scroll_height = scroller->Bounds().Height();
			float diff = settings_height - scroll_height;
			if ( diff < 0 ) diff = 0;
			
			scroller->ScrollBar(B_VERTICAL)->SetRange(0, diff);

			view = scroller;
					
			fPrefViews[protocol] = view;
			fBox->AddChild(view);
			if ( fBox->CountChildren() > 1 ) {
				view->Hide();
			} else {
				fBox->SetLabel(protocol);
				fCurrentView = view;
				fCurrentIndex = fListView->IndexOf(item);
				fListView->Select(fCurrentIndex);
			};
		};
	}
	
	
	// CLIENTS
	
	BMessage clients;
	im_get_client_list(&clients);

	IconTextItem *clientItem = new IconTextItem(_T("Clients"), NULL);
	fListView->AddItem(clientItem);
	
	if (clients.FindString("client")) {
		const char *client = NULL;
		
//		FIX ME: Find the location of the im_server programmatically (By app signature?)
		for (int16 i = 0; clients.FindString("client", i, &client) == B_OK; i++) {
			entry_ref ref;
			//protocols.FindRef("ref", i, &ref);	
			
			BMessage client_settings;
			BMessage client_template;
			
			im_load_client_settings( client, &client_settings );
			im_load_client_template( client, &client_template );
			
			if ( client_settings.FindString("app_sig") ) {
				be_roster->FindApp( client_settings.FindString("app_sig"), &ref );
			}

			BBitmap *icon = ReadNodeIcon(BPath(&ref).Path(), kSmallIcon, true);
			IconTextItem *item = new IconTextItem(client, icon);

			if (strcmp(client, "im_server") == 0) {
				IconTextItem *server = new IconTextItem(_T("Server"), icon);
				fListView->AddItem(server, 0);
				fListView->AddUnder(item, server);
			} else {
				fListView->AddUnder(item, clientItem);
			};
			
			client_template.AddString("client", client); // for identification when saving
			
			pair <BMessage, BMessage> p(client_settings, client_template);
			fAddOns[client] = p; //pair<BMessage, BMessage>(settings, tmplate);
			
			BView *view = new BView(frame, client, B_FOLLOW_ALL_SIDES,
				B_WILL_DRAW);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
			view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
			view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			view->SetHighColor(0, 0, 0, 0);
#endif

			float settings_height = BuildGUI(client_template, client_settings, view);
			
			BScrollView * scroller = new BScrollView(
				"scroller", view, B_FOLLOW_ALL,
				0, false, true
			);
			view->ResizeTo( view->Bounds().Width(), settings_height );
			
			float scroll_height = scroller->Bounds().Height();
			float diff = settings_height - scroll_height;
			if ( diff < 0 ) diff = 0;
			
			scroller->ScrollBar(B_VERTICAL)->SetRange(0, diff);

			view = scroller;
				
			fPrefViews[client] = view;
			fBox->AddChild(view);
			if (fBox->CountChildren() > 1 ) {
				view->Hide();
			} else {
				fBox->SetLabel(client);
				fCurrentView = view;
				fCurrentIndex = fListView->IndexOf(item);
				fListView->Select(fCurrentIndex);
			};
		};
	}
	
	BScrollView *scroller = new BScrollView("list scroller", fListView, B_FOLLOW_LEFT |
		B_FOLLOW_BOTTOM, 0, false, true);
	fView->AddChild(scroller);

	frame = fView->Frame();
	frame.InsetBy(kEdgeOffset, kEdgeOffset);
	frame.bottom -= (kEdgeOffset * 2);
	frame.top = frame.bottom - ((fontHeight.descent + fontHeight.leading + fontHeight.ascent));
	frame.left = frame.right - (be_plain_font->StringWidth(_T("Save")) +
		(kControlOffset * 2));

	fSave = new BButton(frame, "Save", _T("Save"), new BMessage(SAVE));
	fView->AddChild(fSave);

	frame.right = frame.left - kControlOffset;
	frame.left = frame.right - (be_plain_font->StringWidth(_T("Revert")) +
		(kControlOffset * 2));

	fRevert = new BButton(frame, "Revert", _T("Revert"), new BMessage(REVERT));
	fView->AddChild(fRevert);
	fRevert->SetEnabled(false);

	fListView->MakeFocus();
	fListView->SetSelectionMessage(new BMessage(LISTCHANGED));
	fListView->SetTarget(this);
	
	Show();
	fView->Show();	
};

bool PWindow::QuitRequested(void) {
	view_map::iterator vIt;
	for (vIt = fPrefViews.begin(); vIt != fPrefViews.end(); vIt++) {
		BView *view = vIt->second;
		if (!view) continue;
		
		for (int32 j = 0; j < view->CountChildren(); j++) {
			BView *child = view->ChildAt(j);
			if (!child) continue;
			
			child->RemoveSelf();
			delete child;
		};
		
		view->RemoveSelf();
		delete view;
	};
	fPrefViews.clear();

	if (fSave) fSave->RemoveSelf();
	delete fSave;
	
	if (fRevert) fRevert->RemoveSelf();
	delete fRevert;
		
	if (fBox) fBox->RemoveSelf();
	delete fBox;
	
	if (fListView) fListView->RemoveSelf();
	delete fListView;
	
	if (fView != NULL) fView->RemoveSelf();
	delete fView;
	
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	
	return true;
};

void PWindow::DispatchMessage( BMessage * msg, BHandler * target )
{
	switch (msg->what) {
		case B_MOUSE_WHEEL_CHANGED: {
			if (target != fListView) {
				float delta_y=0.0f;
				
				msg->FindFloat("be:wheel_delta_y", &delta_y);
				
				fCurrentView->ScrollBy(0, delta_y * 10);
				return;
			}
		} break;
			
		default: {
		}; break;
	}

	BWindow::DispatchMessage(msg,target);
}

void PWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case LISTCHANGED: {
			int32 index = msg->FindInt32("index");
			if (index < 0) return;
			
			IconTextItem *item = (IconTextItem *)fListView->ItemAt(index);
			if (item == NULL) return;
			
			view_map::iterator vIt = fPrefViews.find(item->Text());
			if (vIt == fPrefViews.end()) {
				fListView->Select(fCurrentIndex);
				return;
			};
			
			if (fCurrentView) fCurrentView->Hide();

			fCurrentView = vIt->second;
			fCurrentView->Show();
			fCurrentIndex = index;
			fBox->SetLabel(item->Text());
			
			fView->Invalidate();
		} break;

//		case REVERT: {
//		} break;
		
		case SAVE: {
			BMessage cur;
			BMessage tmplate;
			BMessage settings;
			BMessage reply;
			
			int current = fListView->CurrentSelection();
			if (current < 0) {
				printf("Error, no selection when trying to update\n");
				return;
			};
			
			IconTextItem *item = (IconTextItem *)fListView->ItemAt(current);
			pair<BMessage, BMessage> p = fAddOns[item->Text()];
			
			tmplate = p.second;			
			BView * panel = FindView(item->Text());
			
			for (int i = 0; tmplate.FindMessage("setting", i, &cur) == B_OK; i++) {
				const char *name = cur.FindString("name");
				int32 type = -1;
				
				cur.FindInt32("type", &type);
				
				if ( dynamic_cast<BTextControl*>(panel->FindView(name))) { 
//					Free text
					BTextControl * ctrl = (BTextControl*)panel->FindView(name);
				
					switch (type) {
						case B_STRING_TYPE: {
							settings.AddString(name, ctrl->Text() );
						} break;
						case B_INT32_TYPE: {
							settings.AddInt32(name, atoi(ctrl->Text()) );
						} break;
						default: {
							return;
						};
					};
				} else if (dynamic_cast<BMenuField*>(panel->FindView(name))) {
//					Provided option
					BMenuField * ctrl = (BMenuField*)panel->FindView(name);
					BMenuItem * item = ctrl->Menu()->FindMarked();
					
					if (!item) return;
					
					switch (type) {
						case B_STRING_TYPE: {
							settings.AddString(name, item->Label() );
						} break;
						case  B_INT32_TYPE: {
							settings.AddInt32(name, atoi(item->Label()) );
						} break;
						default: {
							return;
						};
					}
				} else
				if (dynamic_cast<BCheckBox*>(panel->FindView(name))) {
// 					Boolean setting
					BCheckBox * box = (BCheckBox*)panel->FindView(name);
					
					if ( box->Value() == B_CONTROL_ON ) {
						settings.AddBool(name,true);
					} else {
						settings.AddBool(name,false);
					}
				} else if (dynamic_cast<BTextView *>(panel->FindView(name))) {
					BTextView *view = (BTextView *)panel->FindView(name);
					settings.AddString(name, view->Text());
				};
			};

			status_t res = B_ERROR;
			BMessage updMessage(IM::SETTINGS_UPDATED);
			
			if ( tmplate.FindString("protocol") )
			{
				res = im_save_protocol_settings( tmplate.FindString("protocol"), &settings );
				updMessage.AddString("protocol", tmplate.FindString("protocol") );
			} else
			if ( tmplate.FindString("client") )
			{
				res = im_save_client_settings( tmplate.FindString("client"), &settings );
				updMessage.AddString("client", tmplate.FindString("client") );
			} else
			{
				LOG("Preflet", liHigh, "Failed to determine type of settings");
			}
			
			if ( res != B_OK )
			{
				LOG("Preflet", liHigh, "Error when saving settings");
			} else
			{
				fManager->SendMessage( &updMessage );
			}
		} break;
		default: {
			BWindow::MessageReceived(msg);
		};
	};
};

float PWindow::BuildGUI(BMessage viewTemplate, BMessage settings, BView *view) {
	BMessage curr;
	float yOffset = kEdgeOffset + kControlOffset;
	float xOffset = 0;
	
	const float kControlWidth = view->Bounds().Width() - (kEdgeOffset * 2);
	
	for (int i=0; viewTemplate.FindMessage("setting",i,&curr) == B_OK; i++ ) {
		char temp[512];
		
		// get text etc from template
		const char * name = curr.FindString("name");
		const char * desc = curr.FindString("description");
		const char * value = NULL;
		int32 type = -1;
		bool secret = false;
		bool freeText = true;
		bool multiLine = false;
		BView *control = NULL;
		BMenu *menu = NULL;
		
		if ( name != NULL && strcmp(name,"app_sig") == 0 ) {
			// skip app-sig setting
			continue;
		}
		
		if (curr.FindInt32("type", &type) != B_OK) {
			printf("Error getting type for %s, skipping\n", name);
			continue;
		};
		
		switch (type) {
			case B_STRING_TYPE: {
				if (curr.FindString("valid_value")) {
					// It's a "select one of these" setting
					
					freeText = false;
			
					menu = new BPopUpMenu(name);
//					menu->SetDivider(be_plain_font->StringWidth(name) + 10);
					
					for (int j = 0; curr.FindString("valid_value", j); j++) {
						menu->AddItem(new BMenuItem(curr.FindString("valid_value", j),NULL));
					};
					
					value = settings.FindString(name);
					
					if (value) menu->FindItem(value)->SetMarked(true);
				} else {
					// It's a free-text setting
					
					if (curr.FindBool("multi_line", &multiLine) != B_OK) multiLine = false;
					value = settings.FindString(name);
					if (!value) value = curr.FindString("default");
					if (curr.FindBool("is_secret",&secret) != B_OK) secret = false;
				}
			} break;
			case B_INT32_TYPE: {
				if (curr.FindInt32("valid_value")) {
					// It's a "select one of these" setting
					
					freeText = false;
					
					menu = new BPopUpMenu(name);
					
					int32 v = 0;
					for ( int j = 0; curr.FindInt32("valid_value",j,&v) == B_OK; j++ ) {
						sprintf(temp,"%ld", v);
						menu->AddItem(new BMenuItem(temp, NULL));
					};
				} else {
					// It's a free-text (but number) setting
					int32 v = 0;
					if (settings.FindInt32(name,&v) == B_OK) {
						sprintf(temp,"%ld",v);
						value = temp;
					} else if ( curr.FindInt32("default",&v) == B_OK ) {
						sprintf(temp,"%ld",v);
						value = temp;
					}
					if (curr.FindBool("is_secret",&secret) != B_OK) secret = false;
				}
			} break;
			case B_BOOL_TYPE: {
				bool active;
				
				if (settings.FindBool(name, &active) != B_OK) {
					if (curr.FindBool("default", &active) != B_OK) {
						active = false;
					};
				};
			
				control = new BCheckBox(BRect(0, 0, kControlWidth, fFontHeight),
					name, _T(desc), NULL);
				if (active) ((BCheckBox*)control)->SetValue(B_CONTROL_ON);
			} break;			
			default: {
				continue;
			};
		};
		
		if (!value) value = "";
		
		if (!control) {
			if (freeText) {
				if (multiLine == false) {
					control = new BTextControl(
						BRect(0, 0, kControlWidth, fFontHeight), name,
						_T(desc), value, NULL);
					if (secret) {
						((BTextControl *)control)->TextView()->HideTyping(true);
						((BTextControl *)control)->SetText(_T(value));
					};
					((BTextControl *)control)->SetDivider(kDividerWidth);
				} else {
					BRect labelRect(0, 0, kDividerWidth, fFontHeight);
					BStringView *label = new BStringView(labelRect, "NA", _T(desc),
						B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
					view->AddChild(label);
					label->MoveTo(kEdgeOffset, yOffset);

					BRect rect(0, 0, kControlWidth - kDividerWidth, fFontHeight * 4);
					rect.right -= B_V_SCROLL_BAR_WIDTH + kEdgeOffset + kControlOffset;
					BRect textRect = rect;
					textRect.InsetBy(kEdgeOffset, kEdgeOffset);
					textRect.OffsetTo(1.0, 1.0);

					xOffset = kEdgeOffset + kDividerWidth;
					BTextView *textView = new BTextView(rect, name, textRect,
						B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

					control = new BScrollView("NA", textView, B_FOLLOW_ALL_SIDES,
						B_WILL_DRAW | B_NAVIGABLE, false, true);
					textView->SetText(_T(value));			
				};
			} else {
				control = new BMenuField(BRect(0, 0, kControlWidth, fFontHeight),
					name, _T(desc), menu);
				((BMenuField *)control)->SetDivider(kDividerWidth);
			};
		};
		
		if ( curr.FindString("help") )
		{
			gHelper.SetHelp(control, strdup(curr.FindString("help")));
		}
		
		view->AddChild(control);
			
		float h, w = 0;
		control->GetPreferredSize(&w, &h);
		control->MoveTo(kEdgeOffset + xOffset, yOffset);
		yOffset += kControlOffset + h;
		xOffset = 0;
	};
	
	if ( yOffset < view->Bounds().Height() )
		yOffset = view->Bounds().Height();
	
	return yOffset;//view->ResizeTo( view->Bounds().Width(), yOffset );
};
