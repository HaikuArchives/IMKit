/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif
#include <interface/OutlineListView.h>
#include <interface/ScrollView.h>
#include <interface/Button.h>
#include <interface/TextControl.h>
#include <interface/Screen.h>
#include <interface/Window.h>
#include <storage/Path.h>

#include "PAccountsView.h"

#ifdef ZETA
#	include <app/Roster.h>
#	include <locale/Locale.h>
#	include <storage/Path.h>
#else
#	define _T(str) (str)
#endif

#ifndef B_AUTO_UPDATE_SIZE_LIMITS
#	define B_AUTO_UPDATE_SIZE_LIMITS 0
#endif

#ifndef B_CLOSE_ON_ESCAPE
#	define B_CLOSE_ON_ESCAPE 0
#endif

const int32 kProtocolListChanged = 'Mplc';
const int32 kAddAccount  = 'Mada';
const int32 kEditAccount = 'Meda';
const int32 kDelAccount  = 'Mdea';
const int32 kMsgCancel = 'Mcnl';
const int32 kMsgOk = 'Mokb';

PAccountsView::PAccountsView(BRect bounds, BPath* protoPath)
	: BView(bounds, protoPath->Leaf(), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect frame(0, 0, 1, 1);

	// Create list view
	BOutlineListView* listView = new BOutlineListView(frame, "proto_list", B_MULTIPLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES);
	BMessage* selMsg = new BMessage(kProtocolListChanged);
	selMsg->AddString("protocol", protoPath->Path());
	listView->SetSelectionMessage(selMsg);

	// Create scroll bars
	BScrollView* scrollView = new BScrollView("proto_scroll", listView, B_FOLLOW_ALL, 0, false,
		true, B_FANCY_BORDER);

	// Buttons
	fAddButton = new BButton(frame, "add", _T("Add account..."), new BMessage(kAddAccount));
	fEditButton = new BButton(frame, "edit", _T("Edit account..."), new BMessage(kEditAccount));
	fEditButton->SetEnabled(false);
	fDelButton = new BButton(frame, "del", _T("Remove..."), new BMessage(kDelAccount));
	fDelButton->SetEnabled(false);

#ifdef __HAIKU__
	float inset = ceilf(be_plain_font->Size() * 0.7f);
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
#endif
}


void
PAccountsView::AttachedToWindow()
{
	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fDelButton->SetTarget(this);
}


void
PAccountsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kAddAccount: {
			BRect frame(0, 0, 1, 1);

			BWindow* window = new BWindow(BRect(0, 0, 320, 240), _T("Add account"), B_TITLED_WINDOW,
				B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE);
			BView* view = new BView(frame, "top", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
			view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

			BTextControl* fAccountName = new BTextControl(frame, "account_name", _T("Account name:"), NULL, NULL);

			fCancelButton = new BButton(frame, "cancel", _T("Cancel"), new BMessage(kMsgCancel));
			fOkButton = new BButton(frame, "ok", _T("OK"), new BMessage(kMsgOk));

#ifdef __HAIKU__
			float inset = ceilf(be_plain_font->Size() * 0.7f);
			view->SetLayout(new BGroupLayout(B_VERTICAL));
			view->AddChild(BGroupLayoutBuilder(B_VERTICAL)
				.Add(fAccountName)

				.AddGroup(B_HORIZONTAL, 2.0f)
					.AddGlue()
					.Add(fCancelButton)
					.Add(fOkButton)
				.End()

				.SetInsets(inset, inset, inset, inset)
			);
#else
			view->AddChild(fAccountName);
			view->AddChild(fCancelButton);
			view->AddChild(fOkButton);
#endif

			view->Show();
			window->AddChild(view);
			CenterWindowOnScreen(window);
			window->Show();
		} break;
	}
}


void
PAccountsView::CenterWindowOnScreen(BWindow* window)
{
	BRect screenFrame = BScreen().Frame();
	BPoint pt;

	pt.x = screenFrame.Width()/2 - Bounds().Width()/2;
	pt.y = screenFrame.Height()/2 - Bounds().Height()/2;

	if (screenFrame.Contains(pt))
		window->MoveTo(pt);
}
