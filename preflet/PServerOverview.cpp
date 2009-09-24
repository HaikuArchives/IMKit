/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <interface/StringView.h>
#include <interface/Box.h>
#include <interface/Button.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/GridLayoutBuilder.h>
#	include <interface/SpaceLayoutItem.h>
#endif

#include "PServerOverview.h"
#include <common/interface/Divider.h>

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

//#pragma mark Constructor

PServerOverview::PServerOverview(BRect bounds)
	: AbstractView(bounds, "ServerOverview", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS) {

	float inset = ceilf(be_plain_font->Size() * 0.7f);
	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Frame();
	frame.InsetBy(inset * 2, inset * 2);
#endif

	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);
	
	// Heading
	fServerLabel = new BStringView(frame, "ServerLabel", _T("Server"));
	fServerLabel->SetAlignment(B_ALIGN_LEFT);
	fServerLabel->SetFont(&headingFont);

	fServerDivider = new Divider(frame, "ServerDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fServerDivider->ResizeToPreferred();

#ifdef __HAIKU__
	fServerLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fServerDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(fServerLabel, 0, 0, 2)
			.Add(fServerDivider, 0, 1, 2)
		)

		.AddGlue()
	);
#else
	AddChild(fServerLabel);
	AddChild(fServerDivider);
	
	LayoutGUI();
#endif
}

//#pragma mark BView Hooks

void PServerOverview::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif
};

void PServerOverview::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		
		default: {
			BView::MessageReceived(msg);
		} break;
	};
}

//#pragma mark Private

#ifndef __HAIKU__

void PServerOverview::LayoutGUI(void) {
	font_height fh;
	BFont headingFont(be_bold_font);
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	
	BRect frame = Bounds();
	frame.InsetBy(inset * 2, inset * 2);
	frame.OffsetBy(inset, inset);

	// Server related controls
	fServerLabel->ResizeToPreferred();
	BRect frameServerLabel = fServerLabel->Frame();

	BRect frameServerDivider = fServerDivider->Frame();
	fServerDivider->MoveTo(frameServerDivider.left, frameServerLabel.bottom + inset);
	frameServerDivider = fServerDivider->Frame();
};

#endif
