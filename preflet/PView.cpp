/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GridView.h>
#	include <interface/GridLayout.h>
#	include <interface/GroupView.h>
#	include <interface/SpaceLayoutItem.h>
#endif
#include <interface/OutlineListView.h>
#include <interface/ScrollView.h>
#include <interface/Box.h>
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

#include "PView.h"

const int32 kRevert      = 'REVT';
const int32 kSave        = 'SAVE';
const int32 kListChanged = 'LSCH';

#ifndef __HAIKU__
const float kControlOffset = 5.0;
const float kEdgeOffset = 5.0;
const float kDividerWidth = 100;
#endif

PView::PView()
	: BView("top", 0, NULL)
{
	BRect frame;
	font_height fontHeight;
	BMessage msg;

	// Set background color
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Outline list view
#ifndef __HAIKU__
	frame.left = kEdgeOffset;
	frame.top = kEdgeOffset;
	frame.bottom = Bounds().bottom - kEdgeOffset;
	frame.right = 180;
#endif
	fListView = new BOutlineListView(frame, "listview", B_SINGLE_SELECTION_LIST);
	fListView->MakeFocus();
	fListView->SetSelectionMessage(new BMessage(kListChanged));
	fListView->SetTarget(this);

	// Outline list view scroller
	BScrollView* scroller = new BScrollView("list_scroller", fListView,
		B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, 0, false, true);

	// Add main view
#ifndef __HAIKU__
	frame.left = fListView->Bounds().right + (kEdgeOffset * 3) + B_V_SCROLL_BAR_WIDTH;
	frame.top = kEdgeOffset;
	frame.right = fView->Bounds().right - kEdgeOffset;
	frame.bottom = fView->Bounds().bottom - ((fFontHeight * 2) + kEdgeOffset));
#endif
	fBox = new BBox(frame, "box", B_FOLLOW_ALL_SIDES);
	fBox->SetLabel(_T("IM Server"));

	// Settings item
	IconTextItem* settingsItem = new IconTextItem("settings", _T("Settings"));
	fListView->AddItem(settingsItem);

	// Servers item
	IconTextItem* serversItem = new IconTextItem("servers", _T("Servers"));
	fListView->AddItem(serversItem);

	// Protocols item
	IconTextItem* protocolsItem = new IconTextItem("protocols", _T("Protocols"));
	fListView->AddItem(protocolsItem);

	// Clients item
	IconTextItem* clientsItem = new IconTextItem("clients", _T("Clients"));
	fListView->AddItem(clientsItem);

	// Save and revert buttons
	fRevert = new BButton(frame, "revert", _T("Revert"), new BMessage(kRevert));
	fRevert->SetEnabled(false);
	fSave = new BButton(frame, "save", _T("Save"), new BMessage(kSave));

#ifdef __HAIKU__
	// Root layout
	BGroupLayout* rootLayout = new BGroupLayout(B_VERTICAL);
	SetLayout(rootLayout);

	// Grid layout
	BGridView* controlsGroup = new BGridView();
	BGridLayout* layout = controlsGroup->GridLayout();

	// Insets
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	rootLayout->SetInsets(inset, inset, inset, inset);
	rootLayout->SetSpacing(inset);
	layout->SetSpacing(inset, inset);

	// Add views
	controlsGroup->GridLayout()->AddView(scroller);
	controlsGroup->GridLayout()->AddView(fBox);

	// Buttons group
	BGroupView* buttonsGroup = new BGroupView(B_HORIZONTAL);
	buttonsGroup->GroupLayout()->AddItem(BSpaceLayoutItem::CreateGlue());
	buttonsGroup->GroupLayout()->AddView(fRevert);
	buttonsGroup->GroupLayout()->AddView(fSave);

	rootLayout->AddView(controlsGroup);
	rootLayout->AddView(buttonsGroup);
#else
	AddChild(scroller);
	AddChild(fBox);
	AddChild(fRevert);
	AddChild(fSave);
#endif

	// Adding protocol items
	BMessage protocols;
	im_get_protocol_list(&protocols);

	for (int32 i = 0; protocols.FindMessage("protocol", i, &msg) == B_OK; i++) {
		const char* path;
		const char* file;
		BPath protoPath;
		BView* view;

		// Get protocol add-on path and file
		msg.FindString("path", &path);
		msg.FindString("file", &file);
		protoPath.SetTo(path);
		protoPath.Append(file);

		// Add protocol item
		BBitmap* icon = ReadNodeIcon(protoPath.Path(), B_MINI_ICON, true);
		IconTextItem* item = new IconTextItem(protoPath.Path(), file, icon);
		fListView->AddUnder(item, protocolsItem);
	}

	// Adding client items
	BMessage clients;
	im_get_client_list(&clients);

	for (int32 i = 0; clients.FindMessage("client", i, &msg) == B_OK; i++) {
	}
}
