/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *		Michael Davidson <slaad@bong.com.au>
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

#include "PProtocolsOverview.h"
#include "common/Divider.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

PProtocolsOverview::PProtocolsOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Frame();
	frame.InsetBy(inset * 2, inset * 2);
#endif
	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);

	fProtocolsLabel = new BStringView(frame, "ProtocolsLabel", _T("Protocols"));
	fProtocolsLabel->SetAlignment(B_ALIGN_LEFT);
	fProtocolsLabel->SetFont(&headingFont);

	fProtocolsDivider = new Divider(frame, "ProtocolsDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fProtocolsDivider->ResizeToPreferred();

#ifdef __HAIKU__
	fProtocolsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fProtocolsDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(fProtocolsLabel, 0, 0, 2)
			.Add(fProtocolsDivider, 0, 1, 2)
		)

		.AddGlue()
	);
#else
	AddChild(fProtocolsLabel);
	AddChild(fProtocolsDivider);
	
	LayoutGUI();
#endif
}

//#pragma mark BView Hooks

void PProtocolsOverview::AttachedToWindow(void) {
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

void PProtocolsOverview::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		default: {
			BView::MessageReceived(msg);
		} break;
	};
};

//#pragma mark Private

#ifndef __HAIKU__
void PProtocolsOverview::LayoutGUI(void) {
	font_height fh;
	BFont headingFont(be_bold_font);
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	
	BRect frame = Bounds();
	frame.InsetBy(inset * 2, inset * 2);
	frame.OffsetBy(inset, inset);

	// Protocols related controls
	fProtocolsLabel->ResizeToPreferred();
	BRect frameProtocolsLabel = fProtocolsLabel->Frame();

	BRect frameProtocolsDivider = fProtocolsDivider->Frame();
	fProtocolsDivider->MoveTo(frameProtocolsDivider.left, frameProtocolsLabel.bottom + inset);
	frameProtocolsDivider = fProtocolsDivider->Frame();
};
#endif
