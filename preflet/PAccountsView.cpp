/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>

#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif
#include <interface/OutlineListView.h>
#include <interface/ScrollView.h>
#include <interface/Button.h>
#include <storage/Path.h>

#include "PAccountsView.h"
#include "PAccountDialog.h"

#ifdef ZETA
#	include <app/Roster.h>
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

//#pragma mark Constants

const int32 kProtocolListChanged = 'Mplc';
const int32 kAddAccount = 'Mada';
const int32 kEditAccount = 'Meda';
const int32 kDelAccount = 'Mdea';

//#pragma mark Constructor

PAccountsView::PAccountsView(BRect bounds, BPath* protoPath)
	: BView(bounds, protoPath->Leaf(), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fProtoPath(protoPath)
{
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Frame();
	frame.InsetBy(inset * 2, inset * 2);
#endif

	// Create list view
	fProtocolListView = new BOutlineListView(frame, "ProtocolList", B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	BMessage *selection = new BMessage(kProtocolListChanged);
	selection->AddString("protocol", protoPath->Path());
	fProtocolListView->SetSelectionMessage(selection);

	// Create scroll bars
	BScrollView* scrollView = new BScrollView("ProtocolListScroll", fProtocolListView, B_FOLLOW_ALL, 0, false, true, B_FANCY_BORDER);

	// Buttons
	fAddButton = new BButton(frame, "Add", _T("Add account..."), new BMessage(kAddAccount));
	fEditButton = new BButton(frame, "Edit", _T("Edit account..."), new BMessage(kEditAccount));
	fEditButton->SetEnabled(false);
	fDelButton = new BButton(frame, "Del", _T("Remove..."), new BMessage(kDelAccount));
	fDelButton->SetEnabled(false);

#ifdef __HAIKU__
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_HORIZONTAL, inset)
		.Add(scrollView)

		.AddGroup(B_VERTICAL, 2.0f)
			.Add(fAddButton)
			.Add(fEditButton)
			.Add(fDelButton)
			.AddGlue()
		.End()
	);
#else
	AddChild(scrollView);
	
	LayoutGUI();
#endif
}

//#pragma mark BView Hooks

void PAccountsView::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif

	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fDelButton->SetTarget(this);
}

void PAccountsView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kAddAccount: {
			PAccountDialog* dialog = new PAccountDialog(_T("Add account"), fProtoPath);
			if (dialog->Go() == 1) {
				printf("%s\n", dialog->AccountName());
			}

		} break;
	}
}

//#pragma mark Private

#ifndef __HAIKU__

void PAccountsView::LayoutGUI(void) {
};

#endif
