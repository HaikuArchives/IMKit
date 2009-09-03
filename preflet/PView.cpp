/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <app/Roster.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/GridLayoutBuilder.h>
#	include <interface/LayoutUtils.h>
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

#include <libim/Helpers.h>
#include <libim/Constants.h>

#include "PView.h"
#include "PSettingsOverview.h"
#include "PServerOverview.h"
#include "PProtocolsOverview.h"
#include "PClientsOverview.h"
#include "PClientView.h"
#include "PAccountsView.h"
#include "PUtils.h"
#include "SettingsController.h"
#include "SettingsInfo.h"

//#pragma mark Constants

const int32 kRevert = 'Mrvt';
const int32 kSave = 'Msav';
const int32 kListChanged = 'Mlsc';
const float kEdgeOffset  = 5.0f;
const float kControlOffset = 5.0f;

//#pragma mark Constructors

PView::PView(BRect bounds)
	: AbstractView(bounds, "top", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fCurrentView(NULL),
	fCurrentIndex(0) {
	
	BRect frame;
	font_height fontHeight;

	be_bold_font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.descent + fontHeight.leading + fontHeight.ascent;

	// Set background color
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// IM Manager
	fManager = new IM::Manager(BMessenger(this));

	// Outline list view
#ifdef __HAIKU__
	frame = BRect(0, 0, 1, 1);
#else
	frame = BRect(kEdgeOffset, kEdgeOffset, 180, Bounds().bottom - kEdgeOffset);
#endif

	fListView = new BOutlineListView(BRect(0, 0, 1, 1), "listview", B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES);
	fListView->SetSelectionMessage(new BMessage(kListChanged));
	fListView->MakeFocus();
#ifdef __HAIKU__
	BScrollView* listScroller = new BScrollView("listview_scroll", fListView, 0,
		0, false, true, B_FANCY_BORDER);
#else
	BScrollView* listScroller = new BScrollView("listview_scroll", fListView, B_FOLLOW_ALL,
		0, false, true, B_FANCY_BORDER);
#endif

	// Add main view
#ifndef __HAIKU__
	frame.left = fListView->Bounds().right + (kEdgeOffset * 3) + B_V_SCROLL_BAR_WIDTH;
	frame.top = kEdgeOffset;
	frame.right = Bounds().right - kEdgeOffset;
	frame.bottom = Bounds().bottom - ((fFontHeight * 2) + kEdgeOffset);
#endif
	fMainView = new PCardView(frame);

	BRect frameSave(frame);
	BRect frameRevert(frame);

#ifndef __HAIKU__
	frameSave = Bounds();

	frameSave.InsetBy(kEdgeOffset, kEdgeOffset);
	frameSave.bottom -= (kEdgeOffset * 2);
	frameSave.top = frameSave.bottom - ((fontHeight.descent + fontHeight.leading + fontHeight.ascent));
	frameSave.left = frameSave.right - (be_plain_font->StringWidth(_T("Save")) + (kControlOffset * 2));

	frameRevert.top = frameSave.top;
	frameRevert.bottom = frameSave.bottom;
	frameRevert.right = frameSave.left - kControlOffset;
	frameRevert.left = frameRevert.right - (be_plain_font->StringWidth(_T("Revert")) + (kControlOffset * 2));
#endif

	// Save and revert buttons
	fRevert = new BButton(frameRevert, "revert", _T("Revert"), new BMessage(kRevert));
	fRevert->SetEnabled(false);
	fSave = new BButton(frameSave, "save", _T("Save"), new BMessage(kSave));
	fSave->SetEnabled(false);

	// Settings item
	IconTextItem* settingsItem = new IconTextItem("settings", _T("Settings"));
	fListView->AddItem(settingsItem);
	fViews["settings"] = new PSettingsOverview(this, fMainView->Bounds());
	fMainView->Add(fViews["settings"]);
	fCurrentView = fViews["settings"];
	fCurrentIndex = fListView->IndexOf(settingsItem);
	fListView->Select(fCurrentIndex);
	fMainView->Select(fCurrentView);

	// Clients item
	fClientsItem = new IconTextItem("clients", _T("Clients"));
	fListView->AddUnder(fClientsItem, settingsItem);
	fViews["clients"] = new PClientsOverview(this, fMainView->Bounds());
	dynamic_cast<SettingsController *>(fViews["clients"])->Init(this);
	fMainView->Add(fViews["clients"]);

	// Protocols item
	fProtocolsItem = new IconTextItem("protocols", _T("Protocols"));
	fListView->AddUnder(fProtocolsItem, settingsItem);
	fViews["protocols"] = new PProtocolsOverview(fMainView->Bounds());
	fMainView->Add(fViews["protocols"]);

	// Server item
	fServerItem = new IconTextItem("server", _T("Server"));
	fListView->AddUnder(fServerItem, settingsItem);
	fViews["server"] = new PServerOverview(fMainView->Bounds());
	fMainView->Add(fViews["server"]);

	// Add protocols and clients
	LoadProtocols();
	LoadClients();

#ifdef __HAIKU__
	// Calculate inset
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	// Build the layout
	SetLayout(new BGroupLayout(B_VERTICAL));

	AddChild(BGroupLayoutBuilder(B_HORIZONTAL, inset)
		.Add(listScroller, 0.1f)
		.Add(fMainView, 0.9f)
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
};

PView::~PView(void) {
	for (addons_map::iterator aIt = fAddOns.begin(); aIt != fAddOns.end(); aIt++) {
		delete aIt->second;
	};
	
	fAddOns.clear();
};

//#pragma mark BView Hooks

BSize PView::MinSize() {
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
		BSize(500, 500));
};

BSize PView::MaxSize() {
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(640, 480));
};

void PView::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif

	fListView->SetTarget(this);
	fRevert->SetTarget(this);
	fSave->SetTarget(this);
}

void PView::MessageReceived(BMessage* msg) {
	switch (msg->what) {
		case kListChanged: {
			int32 index = B_ERROR;
			if (msg->FindInt32("index", &index) != B_OK) return;

			IconTextItem* item = (IconTextItem *)fListView->FullListItemAt(index);
			if (item == NULL) return;

			view_map::iterator vIt = fViews.find(item->Name());
			if (vIt == fViews.end()) {
				fListView->Select(fCurrentIndex);
				return;
			};

			fCurrentView = vIt->second;
			fMainView->Select(fCurrentView);
			fCurrentIndex = index;
		} break;

		case kSave:
			SaveSettings();
			break;

		case kRevert:
			RevertSettings();
			break;

		default:
			BView::MessageReceived(msg);
			break;
	}
}

//#pragma mark MultipleViewHandler Hooks

void PView::ShowServerOverview(void) {
	fListView->Select(fListView->IndexOf(fServerItem));
};

void PView::ShowProtocolsOverview(void) {
	fListView->Select(fListView->IndexOf(fProtocolsItem));
};

void PView::ShowClientsOverview(void) {
	fListView->Select(fListView->IndexOf(fClientsItem));
};

//#pragma mark SettingsHost Hooks

void PView::ControllerModified(SettingsController *controler) {
	fSave->SetEnabled(true);
	fRevert->SetEnabled(true);
};

//#pragma mark Private

void PView::LoadProtocols(void) {
	BMessage msg;
	BMessage protocols;
	im_get_protocol_list(&protocols);

	// Adding protocol items
	for (int32 i = 0; protocols.FindMessage("protocol", i, &msg) == B_OK; i++) {
		const char* path;
		const char* file;
		BPath protoPath;

		// Get protocol add-on path and file
		msg.FindString("path", &path);
		msg.FindString("file", &file);
		protoPath.SetTo(path);
		protoPath.Append(file);

		// Add protocol item
		BBitmap* icon = ReadNodeIcon(protoPath.Path(), B_MINI_ICON, true);
		IconTextItem* item = new IconTextItem(protoPath.Path(), file, icon);
		fListView->AddUnder(item, fProtocolsItem);

		BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
		frame = fMainView->Bounds();
#endif

		// Create protocol settings view
		PAccountsView *view = new PAccountsView(frame, &protoPath);
		BMessage tmplate = view->SettingsTemplate();
		SettingsController *controller = dynamic_cast<SettingsController *>(view);
		controller->Init(this);

		// Add protocol settings view
		fMainView->Add(view);
		fViews[protoPath.Path()] = view;

		// Store the template
		SettingsInfo *info = new SettingsInfo(Protocol, protoPath, protoPath.Leaf());
		info->Controller(controller);
		info->View(view);
		info->Template(tmplate);

		fAddOns[protoPath.Path()] = info;
	}
}


void PView::LoadClients(void) {
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
		};

		// Add client item
		BBitmap* icon = ReadNodeIcon(BPath(&ref).Path(), kSmallIcon, true);
		IconTextItem* item = new IconTextItem(clientPath.Path(), file, icon);

		// Add im_server
		if (strcmp(file, "im_server") == 0) {
			fListView->AddUnder(item, fServerItem);
		} else {
			fListView->AddUnder(item, fClientsItem);
		};

		// Create settings view
		BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
		frame = fMainView->Bounds();
		frame.InsetBy(kEdgeOffset, kEdgeOffset);
		frame.top += fFontHeight;
		frame.right -= B_V_SCROLL_BAR_WIDTH + 2;
#endif

		PClientView *view = new PClientView(frame, clientPath.Path(), file, client_template, client_settings);
		SettingsController *controller = dynamic_cast<SettingsController *>(view);
		controller->Init(this);
		fViews[clientPath.Path()] = view;
		fMainView->Add(view);
		
		SettingsInfo *info = new SettingsInfo(Client, clientPath, clientPath.Leaf());
		info->Controller(controller);
		info->View(view);
		info->Settings(client_settings);
		info->Template(client_template);
		fAddOns[clientPath.Path()] = info;
	}
}

void PView::SaveSettings(void) {
	// Loop over all the list view items
	for (int32 i = 0; i < fListView->CountItems(); i++) {
		IconTextItem *item = dynamic_cast<IconTextItem *>(fListView->ItemAt(i));
		BMessage settingsMsg;

		// Get the Template Message
		SettingsInfo *info = fAddOns[item->Name()];
		if (info == NULL) continue;
		BMessage templateMsg = info->Template();

		// Find the right settings controller
		BView *view = FindView(item->Name());

		// Skip this item if can't find the view
		if (view == NULL) {
			continue;
		};

		SettingsController *controller = dynamic_cast<SettingsController *>(view);

		// Save settings
		status_t res = controller->Save(&templateMsg, &settingsMsg);

		if (res == B_OK && fManager->InitCheck() == B_OK) {
			BMessage updMessage(IM::SETTINGS_UPDATED);
			const char *context = NULL;
			
			templateMsg.PrintToStream();

			switch (info->Type()) {
				case Protocol: {
					context = info->Name();
					res = im_save_protocol_settings(context, &settingsMsg);
					updMessage.AddString("protocol", context);
				} break;
				
				case Client: {
					context = info->Name();
					res = im_save_client_settings(context, &settingsMsg);
					updMessage.AddString("client", context);
				} break;
					
				default: {
					LOG("Preflet", liHigh, "Failed to determine type of settings");
				} break;
			};

			if ((res != B_OK) || (context == NULL)) {
				LOG("Preflet", liHigh, "An error occured when saving settings or the setting type could not be determined");
			} else {
				fManager->SendMessage(&updMessage);
			};
		};
	};

	fRevert->SetEnabled(false);
	fSave->SetEnabled(false);
}

void PView::RevertSettings(void) {
	// Loop over all the list view items
	for (int32 i = 0; i < fListView->CountItems(); i++) {
		IconTextItem *item = dynamic_cast<IconTextItem *>(fListView->ItemAt(i));

		// Find the right settings controller
		BView *view = FindView(item->Name());

		// Skip this item if can't find the view
		if (view == NULL) {
			continue;
		};
		
		SettingsInfo *info = fAddOns[item->Name()];
		if (info == NULL) continue;
		BMessage templateMsg = info->Template();
		
		SettingsController *controller = dynamic_cast<SettingsController *>(view);

		// Revert settings
		controller->Revert(&templateMsg);
	};

	fRevert->SetEnabled(false);
	fSave->SetEnabled(false);
};

