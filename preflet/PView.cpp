/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <app/Roster.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/GridLayoutBuilder.h>
#endif
#include <interface/Box.h>
#include <interface/OutlineListView.h>
#include <interface/ScrollView.h>
#include <interface/CheckBox.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <interface/MenuField.h>
#include <interface/PopUpMenu.h>
#include <interface/TextControl.h>
#include <interface/TextView.h>
#include <interface/StringView.h>
#include <storage/Path.h>
#include <storage/Mime.h>

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

#include <common/IconTextItem.h>
#include <common/IMKitUtilities.h>
#include <common/BubbleHelper.h>

#include <libim/Helpers.h>
#include <libim/Constants.h>

#include "PView.h"
#include "PSettingsOverview.h"
#include "PServersOverview.h"
#include "PProtocolsOverview.h"
#include "PClientsOverview.h"

const int32 kRevert      = 'Mrvt';
const int32 kSave        = 'Msav';
const int32 kListChanged = 'Mlsc';

const float kControlOffset = 5.0;
const float kEdgeOffset = 5.0;
const float kDividerWidth = 100;

BubbleHelper gHelper;

PView::PView(BRect bounds)
	: BView(bounds, "top", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fCurrentView(NULL),
	fCurrentIndex(0)
{
	BRect frame;
	font_height fontHeight;

	be_bold_font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.descent + fontHeight.leading + fontHeight.ascent;

	// Set background color
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Calculate inset
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	// IM Manager
	fManager = new IM::Manager(BMessenger(this));

	// Outline list view
#ifdef __HAIKU__
	frame = BRect(0, 0, 1, 1);
#else
	frame = BRect(kEdgeOffset, kEdgeOffset, 180, Bounds().bottom - kEdgeOffset);
#endif
	fListView = new BOutlineListView(frame, "listview", B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES);
	fListView->SetSelectionMessage(new BMessage(kListChanged));
	fListView->MakeFocus();
	BScrollView* listScroller = new BScrollView("listview_scroll", fListView, B_FOLLOW_ALL,
		0, false, true, B_FANCY_BORDER);

	// Add main view
#ifdef __HAIKU__
	frame = BRect(0, 0, 1, 1);
#else
	frame = BRect(fListView->Bounds().right + (kEdgeOffset * 3) + B_V_SCROLL_BAR_WIDTH,
		kEdgeOffset, Bounds().right - kEdgeOffset, Bounds().bottom - ((fFontHeight * 2) + kEdgeOffset));
#endif
	fMainView = new BView(frame, "box", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	// Save and revert buttons
	fRevert = new BButton(frame, "revert", _T("Revert"), new BMessage(kRevert));
	fRevert->SetEnabled(false);
	fSave = new BButton(frame, "save", _T("Save"), new BMessage(kSave));

	// Settings item
	IconTextItem* settingsItem = new IconTextItem("settings", _T("Settings"));
	fListView->AddItem(settingsItem);
	fViews["settings"] = new PSettingsOverview(fMainView->Bounds());
	fMainView->AddChild(fViews["settings"]);
	fCurrentView = fViews["settings"];
	fCurrentIndex = fListView->IndexOf(settingsItem);
	fListView->Select(fCurrentIndex);

	// Servers item
	fServersItem = new IconTextItem("servers", _T("Servers"));
	fListView->AddItem(fServersItem);
	fViews["servers"] = new PServersOverview(fMainView->Bounds());
	fMainView->AddChild(fViews["servers"]);
	fViews["servers"]->Hide();

	// Protocols item
	fProtocolsItem = new IconTextItem("protocols", _T("Protocols"));
	fListView->AddItem(fProtocolsItem);
	fViews["protocols"] = new PProtocolsOverview(fMainView->Bounds());
	fMainView->AddChild(fViews["protocols"]);
	fViews["protocols"]->Hide();

	// Clients item
	fClientsItem = new IconTextItem("clients", _T("Clients"));
	fListView->AddItem(fClientsItem);
	fViews["clients"] = new PClientsOverview(fMainView->Bounds());
	fMainView->AddChild(fViews["clients"]);
	fViews["clients"]->Hide();

	// Add protocols and clients
	LoadProtocols();
	LoadClients();

#ifdef __HAIKU__
	// Build the layout
	SetLayout(new BGroupLayout(B_VERTICAL));

	AddChild(BGroupLayoutBuilder(B_HORIZONTAL, inset)
		.Add(listScroller, 0.3f)
		.Add(fMainView, 0.7f)
		.SetInsets(inset, inset, inset, inset)
	);
	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.AddGroup(B_HORIZONTAL, inset)
			.AddGlue()
			.Add(fRevert)
			.Add(fSave)
		.End()
		.SetInsets(inset, inset, inset, inset)
	);
#else
	AddChild(listScroller);
	AddChild(fMainView);
	AddChild(fRevert);
	AddChild(fSave);
#endif
}


void
PView::AttachedToWindow()
{
	fListView->SetTarget(this);
	fRevert->SetTarget(this);
	fSave->SetTarget(this);
}


void
PView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kListChanged: {
			int32 index = msg->FindInt32("index");
			if (index < 0)
				return;

			IconTextItem* item = (IconTextItem *)fListView->ItemAt(index);
			if (item == NULL)
				return;

			view_map::iterator vIt = fViews.find(item->Name());
			if (vIt == fViews.end()) {
				fListView->Select(fCurrentIndex);
				return;
			}

			if (fCurrentView != NULL)
				fCurrentView->Hide();
			fCurrentView = vIt->second;
			fCurrentView->Show();
			fCurrentIndex = index;
		} break;

		case kMsgEditServers:
			fListView->Select(fListView->IndexOf(fServersItem));
			break;

		case kMsgEditProtocols:
			fListView->Select(fListView->IndexOf(fProtocolsItem));
			break;

		case kMsgEditClients:
			fListView->Select(fListView->IndexOf(fClientsItem));
			break;

		case kSave: {
			BMessage cur;
			BMessage tmplate;
			BMessage settings;
			BMessage reply;

			int current = fListView->CurrentSelection();
			if (current < 0) {
				printf("Error, no selection when trying to update\n");
				return;
			}

			IconTextItem* item = (IconTextItem *)fListView->ItemAt(current);
			addons_pair p = fAddOns[item->Name()];

			tmplate = p.second;			
			BView *panel = FindView(item->Name());

			for (int i = 0; tmplate.FindMessage("setting", i, &cur) == B_OK; i++) {
				const char *name = cur.FindString("name");
				int32 type = -1;

				cur.FindInt32("type", &type);

				if (dynamic_cast<BTextControl*>(panel->FindView(name))) {
					// Free text
					BTextControl* ctrl = (BTextControl*)panel->FindView(name);

					switch (type) {
						case B_STRING_TYPE:
							settings.AddString(name, ctrl->Text());
							break;
						case B_INT32_TYPE:
							settings.AddInt32(name, atoi(ctrl->Text()));
							break;
						default:
							return;
					}
				} else if (dynamic_cast<BMenuField*>(panel->FindView(name))) {
					// Provided option
					BMenuField* ctrl = (BMenuField*)panel->FindView(name);
					BMenuItem* item = ctrl->Menu()->FindMarked();

					if (!item)
						return;

					switch (type) {
						case B_STRING_TYPE:
							settings.AddString(name, item->Label());
							break;
						case  B_INT32_TYPE:
							settings.AddInt32(name, atoi(item->Label()));
							break;
						default:
							return;
					}
				} else if (dynamic_cast<BCheckBox*>(panel->FindView(name))) {
					// Boolean setting
					BCheckBox* box = (BCheckBox*)panel->FindView(name);

					if (box->Value() == B_CONTROL_ON)
						settings.AddBool(name, true);
					else
						settings.AddBool(name, false);
				} else if (dynamic_cast<BTextView*>(panel->FindView(name))) {
					BTextView* view = (BTextView *)panel->FindView(name);
					settings.AddString(name, view->Text());
				}
			}

			status_t res = B_ERROR;
			BMessage updMessage(IM::SETTINGS_UPDATED);

			if (tmplate.FindString("protocol")) {
				res = im_save_protocol_settings(tmplate.FindString("protocol"), &settings);
				updMessage.AddString("protocol", tmplate.FindString("protocol"));
			} else if (tmplate.FindString("client")) {
				res = im_save_client_settings( tmplate.FindString("client"), &settings);
				updMessage.AddString("client", tmplate.FindString("client"));
			} else
				LOG("Preflet", liHigh, "Failed to determine type of settings");

			if (res != B_OK)
				LOG("Preflet", liHigh, "Error when saving settings");
			else
				fManager->SendMessage(&updMessage);
		}

		default:
			BView::MessageReceived(msg);
	}
}


void
PView::LoadProtocols()
{
	BMessage msg;

	// Adding protocol items
	BMessage protocols;
	im_get_protocol_list(&protocols);

	for (int32 i = 0; protocols.FindMessage("protocol", i, &msg) == B_OK; i++) {
		const char* path;
		const char* file;
		BPath protoPath;

		// Get protocol add-on path and file
		msg.FindString("path", &path);
		msg.FindString("file", &file);
		protoPath.SetTo(path);
		protoPath.Append(file);

		// Load settings
		BMessage protocol_settings;
		im_load_protocol_settings(protoPath.Path(), &protocol_settings);

		// Load template
		BMessage protocol_template;
		im_load_protocol_template(protoPath.Path(), &protocol_template);
		protocol_template.AddString("protocol", protoPath.Path());

		// Add protocol item
		BBitmap* icon = ReadNodeIcon(protoPath.Path(), B_MINI_ICON, true);
		IconTextItem* item = new IconTextItem(protoPath.Path(), file, icon);
		fListView->AddUnder(item, fProtocolsItem);

		BView* view = new BView(BRect(0, 0, 1, 1), file, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
		BuildGUI(protocol_template, protocol_settings, file, view);
		fViews[protoPath.Path()] = view;
		fMainView->AddChild(view);
		view->Hide();
	}
}


void
PView::LoadClients()
{
	BMessage msg;

	// Adding client items
	BMessage clients;
	im_get_client_list(&clients);

	for (int32 i = 0; clients.FindMessage("client", i, &msg) == B_OK; i++) {
		const char* path;
		const char* file;
		BPath clientPath;
		entry_ref ref;

		// Get client path and file
		msg.FindString("path", &path);
		msg.FindString("file", &file);
		clientPath.SetTo(path);
		clientPath.Append(file);

		// Load settings
		BMessage client_settings;
		im_load_client_settings(file, &client_settings);

		// Load template
		BMessage client_template;
		im_load_client_template(file, &client_template);

		if (client_settings.FindString("app_sig")) {
			be_roster->FindApp(client_settings.FindString("app_sig"), &ref);
			client_template.AddString("client", file);
		}

		// Add client item
		BBitmap* icon = ReadNodeIcon(BPath(&ref).Path(), kSmallIcon, true);
		IconTextItem* item = new IconTextItem(clientPath.Path(), file, icon);

		// Add im_server
		if (strcmp(file, "im_server") == 0)
			fListView->AddUnder(item, fServersItem);
		else
			fListView->AddUnder(item, fClientsItem);

		pair<BMessage, BMessage> p(client_settings, client_template);
		fAddOns[clientPath.Path()] = p;

		// Create settings view
		BView* view = new BView(BRect(0, 0, 1, 1), file, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
		BuildGUI(client_template, client_settings, file, view);
		fViews[clientPath.Path()] = view;
		fMainView->AddChild(view);
		view->Hide();
	}
}


float
PView::BuildGUI(BMessage templ, BMessage settings, const char* viewName, BView* view)
{
	BMessage curr;
#ifdef __HAIKU__
	// Setup layout
	float inset = ceilf(be_plain_font->Size() * 0.7);
	view->SetLayout(new BGroupLayout(B_VERTICAL));
	BGroupLayoutBuilder layout(B_VERTICAL, inset);
#else
	float yOffset = kEdgeOffset + kControlOffset;
	float xOffset = 0;

	const float kControlWidth = view->Bounds().Width() - (kEdgeOffset * 2);
#endif

#ifdef __HAIKU__
	BStringView* descLabel = new BStringView(BRect(0, 0, 1, 1), NULL, viewName);
	descLabel->SetAlignment(B_ALIGN_LEFT);
	descLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BBox* divider = new BBox(BRect(0, 0, 1, 1), B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	layout.Add(descLabel);
	layout.Add(divider);
#endif

	for (int32 i = 0; templ.FindMessage("setting", i, &curr) == B_OK; i++) {
		char temp[512];

		// Get stuff from templ
		const char* name = curr.FindString("name");
		const char* desc = curr.FindString("description");
		const char* value = NULL;
		int32 type = -1;
		bool secret = false;
		bool freeText = true;
		bool multiLine = false;
		BView* control = NULL;
		BMenu* menu = NULL;
		BRect frame(0, 0, 1, 1);

		if (name != NULL && strcmp(name,"app_sig") == 0) {
			// skip app-sig setting
			continue;
		}

		if (curr.FindInt32("type", &type) != B_OK) {
			LOG("preflet", liMedium, "Error getting type for %s, skipping", name);
			continue;
		}

		switch (type) {
			case B_STRING_TYPE: {
				if (curr.FindString("valid_value")) {
					// It's a "select one of these" setting

					freeText = false;

					menu = new BPopUpMenu(name);		
					for (int j = 0; curr.FindString("valid_value", j); j++)
						menu->AddItem(new BMenuItem(curr.FindString("valid_value", j), NULL));

					value = settings.FindString(name);
					if (value)
						menu->FindItem(value)->SetMarked(true);
				} else {
					// It's a free-text setting
					if (curr.FindBool("multi_line", &multiLine) != B_OK)
						multiLine = false;
					value = settings.FindString(name);
					if (!value)
						value = curr.FindString("default");
					if (curr.FindBool("is_secret",&secret) != B_OK)
						secret = false;
				}
			} break;
			case B_INT32_TYPE: {
				if (curr.FindInt32("valid_value")) {
					// It's a "select one of these" setting

					freeText = false;

					menu = new BPopUpMenu(name);

					int32 def = 0;
					status_t hasValue = settings.FindInt32(name, 0, &def);

					if (hasValue != B_OK)
						hasValue = curr.FindInt32("default", 0, &def);

					int32 v = 0;
					for (int j = 0; curr.FindInt32("valid_value",j,&v) == B_OK; j++) {
						sprintf(temp,"%ld", v);

						BMenuItem* item = new BMenuItem(temp, NULL);

						if (hasValue != B_OK && j == 0)
							item->SetMarked(true);
						else if (hasValue == B_OK && def == v)
							item->SetMarked(true);

						menu->AddItem(item);
					}
				} else {
					// It's a free-text (but number) setting
					int32 v = 0;
					if (settings.FindInt32(name, &v) == B_OK) {
						sprintf(temp,"%ld", v);
						value = temp;
					} else if (curr.FindInt32("default", &v) == B_OK) {
						sprintf(temp,"%ld", v);
						value = temp;
					}
					if (curr.FindBool("is_secret",&secret) != B_OK)
						secret = false;
				}
			} break;
			case B_BOOL_TYPE: {
				bool active;

				if (settings.FindBool(name, &active) != B_OK) {
					if (curr.FindBool("default", &active) != B_OK) {
						active = false;
					}
				}

#ifndef __HAIKU__
				frame = BRect(0, 0, kControlWidth, fFontHeight);
#endif
				control = new BCheckBox(frame, name, _T(desc), NULL);
				if (active)
					((BCheckBox*)control)->SetValue(B_CONTROL_ON);
			} break;
			default:
				continue;
		}

		if (!value)
			value = "";

		if (!control) {
			if (freeText) {
				if (!multiLine) {
#ifndef __HAIKU__
					frame = BRect(0, 0, kControlWidth, fFontHeight);
#endif
					control = new BTextControl(frame, name, _T(desc), value, NULL);
					if (secret) {
						((BTextControl*)control)->TextView()->HideTyping(true);
						((BTextControl*)control)->SetText(_T(value));
					}
					((BTextControl*)control)->SetDivider(kDividerWidth);
				} else {
#ifndef __HAIKU__
					frame  = BRect(0, 0, kDividerWidth, fFontHeight);
#endif
					BStringView* label = new BStringView(frame, "NA", _T(desc), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
#ifdef __HAIKU__
					layout.Add(label);
#else
					view->AddChild(label);
					label->MoveTo(kEdgeOffset, yOffset);

					frame = BRect(0, 0, kControlWidth - kDividerWidth, fFontHeight * 4);
					frame.right -= B_V_SCROLL_BAR_WIDTH + kEdgeOffset + kControlOffset;

					xOffset = kEdgeOffset + kDividerWidth;
#endif

					BRect textRect = frame;
					textRect.InsetBy(kEdgeOffset, kEdgeOffset);
					textRect.OffsetTo(1.0, 1.0);

					BTextView *textView = new BTextView(frame, name, textRect, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

					control = new BScrollView("NA", textView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_NAVIGABLE, false, true);
					textView->SetText(_T(value));			
				}
			} else {
#ifndef __HAIKU__
				frame = BRect(0, 0, kControlWidth, fFontHeight);
#endif
				control = new BMenuField(frame, name, _T(desc), menu);
				((BMenuField *)control)->SetDivider(kDividerWidth);
			}
		}

		if (curr.FindString("help"))
			gHelper.SetHelp(control, strdup(curr.FindString("help")));

#ifdef __HAIKU__
		layout.Add(control);
#else
		view->AddChild(control);
#endif

#ifndef __HAIKU__
		float h, w = 0;
		control->GetPreferredSize(&w, &h);
		control->MoveTo(kEdgeOffset + xOffset, yOffset);
		yOffset += kControlOffset + h;
		xOffset = 0;
#endif
	}

#ifdef __HAIKU__
	layout.AddGlue();
	view->AddChild(layout);

	return 0.0f;
#else
	if (yOffset < view->Bounds().Height())
		yOffset = view->Bounds().Height();

	return yOffset;
#endif
}
