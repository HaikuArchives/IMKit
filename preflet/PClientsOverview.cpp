/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <interface/StringView.h>
#include <interface/TextView.h>
#include <interface/Box.h>
#include <interface/Button.h>
#include <interface/CheckBox.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/GridLayoutBuilder.h>
#	include <interface/SpaceLayoutItem.h>
#endif
#include <storage/Path.h>
#include <libim/Helpers.h>

#include "PClientsOverview.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

PClientsOverview::PClientsOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
#ifdef __HAIKU__
	BRect frame(0, 0, 1, 1);
	float inset = ceilf(be_plain_font->Size() * 0.7f);
#else
	BRect frame;
#endif

	BStringView* autostartLabel = new BStringView(frame, NULL, _T("Autostart"));
#ifdef __HAIKU__
	autostartLabel->SetAlignment(B_ALIGN_LEFT);
	autostartLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	autostartLabel->SetFont(be_bold_font);

	BRect textRect(0, 0, 200, B_SIZE_UNLIMITED);
	BTextView* descLabel = new BTextView(frame, NULL, textRect,
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
	descLabel->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rgb_color textColor = ui_color(B_PANEL_TEXT_COLOR);
	descLabel->SetFontAndColor(be_plain_font, B_FONT_ALL, &textColor);
	descLabel->SetText(_T("Clients set to autostart will start when the Server starts. "
		"This is useful for clients you will always use, such as notifications "
		"or chat windows."));
	descLabel->MakeEditable(false);
	descLabel->MakeSelectable(false);
	descLabel->SetWordWrap(true);
	descLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BBox* divider1 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider1->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
#endif

#ifdef __HAIKU__
	int32 row = 5;
	BGridLayoutBuilder layout(0.0f, 1.0f);
	layout.Add(autostartLabel, 0, 0, 2)
	      .Add(divider1, 0, 1, 2)
	      .Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 2, 2)
	      .Add(descLabel, 0, 3, 2)
	      .Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 4, 2)
	;
#endif

	BMessage clients, msg;
	im_get_client_list(&clients);
	for (int32 i = 0; clients.FindMessage("client", i, &msg) == B_OK; i++) {
		const char* path;
		const char* file;

		// Get client path and file
		msg.FindString("path", &path);
		msg.FindString("file", &file);

		// Autostart checkbox
		BCheckBox* checkbox = new BCheckBox(frame, path, file, NULL);
		checkbox->SetValue(B_CONTROL_ON);

		// Edit button
		BMessage* editMsg = new BMessage(kMsgEditClient);
		editMsg->AddString("path", path);
		editMsg->AddString("file", file);
		BButton* button = new BButton(frame, "edit", _T("Edit..."), editMsg);
		//fEditButtons->Append(button);
#ifdef __HAIKU__
		layout.Add(checkbox, 0, row);
		layout.Add(button, 1, row);
		layout.Add(BSpaceLayoutItem::CreateVerticalStrut(8.0f), 0, ++row, 2);
#endif		
	}

#ifdef __HAIKU__
	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(layout)
		.AddGlue()
		.SetInsets(inset, inset, inset, inset)
	);
#else
	AddChild(autostartLabel);
#endif
}
