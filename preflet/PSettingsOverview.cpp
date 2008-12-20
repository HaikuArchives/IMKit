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

#include "PSettingsOverview.h"

#include "common/Divider.h"
#include "common/MultiLineStringView.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

//#pragma mark Constants

const char *kServerDesc = "The server is responsible for handling contact statuses, loading protocols and communicating between the protocols and clients.";
const char *kProtocolsDesc = "Protocols communicate with instant messaging networks.";

//#pragma mark Constructor

PSettingsOverview::PSettingsOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fServerLabel(NULL),
	fServerDivider(NULL),
	fServerDesc(NULL),
	fServerButton(NULL),
	fProtocolsLabel(NULL),
	fProtocolDivider(NULL),
	fProtocolsDesc(NULL),
	fProtocolsButton(NULL),
	fClientsLabel(NULL),
	fClientsDivider(NULL),
	fClientsDesc(NULL),
	fClientsButton(NULL)
{
	BRect frame(0, 0, 1, 1);
	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);

	font_height fh;
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	
#ifndef __HAIKU__
	frame = Bounds();
frame.PrintToStream();
	frame.InsetBy(inset * 2, inset * 2);
	frame.OffsetBy(inset, inset);
#endif

	BRect frameServerLabel(frame);
#ifndef __HAIKU__
	frameServerLabel.bottom = frameServerLabel.top + headingFontHeight;
#endif

	fServerLabel = new BStringView(frameServerLabel, "ServerLabel", _T("Server"));
	fServerLabel->SetAlignment(B_ALIGN_LEFT);
	fServerLabel->SetFont(&headingFont);

	BRect frameDivider1(frame);
#ifndef __HAIKU__
	frameDivider1.top = frameServerLabel.bottom + inset;
	frameDivider1.bottom = frameDivider1.top + 25.0f;
#endif

	fServerDivider = new Divider(frameDivider1, "divider1", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fServerDivider->ResizeToPreferred();
	frameDivider1 = fServerDivider->Frame();

	BRect frameServerDescLabel(frame);
#ifndef __HAIKU__
	frameServerDescLabel.top = frameDivider1.bottom + inset;
	frameServerDescLabel.bottom = frame.bottom;
#endif

	fServerDesc = new MultiLineStringView(frameServerDescLabel, "ServersDesc", _T(kServerDesc));
	fServerDesc->MoveTo(frameServerDescLabel.left, frameServerDescLabel.top);
	fServerDesc->ResizeToPreferred();
	
	BRect frameServerButton(frame);
#ifndef __HAIKU__
	frameServerButton.top = fServerDesc->Frame().bottom + inset;
	frameServerButton.bottom = frameServerButton.top + 20;
#endif

	fServerButton = new BButton(frameServerButton, "server_edit", _T("Edit..."), new BMessage(kMsgEditServer));
#ifndef __HAIKU__
	fServerButton->ResizeToPreferred();
	BRect frameServerPreferred = fServerButton->Frame();
	fServerButton->MoveTo(frame.right - inset - frameServerPreferred.Width(), frameServerButton.top);
#endif

	BRect frameProtocolsLabel(frame);
#ifndef __HAIKU__
	frameProtocolsLabel.top = frameServerButton.bottom + inset;
	frameProtocolsLabel.bottom = frameProtocolsLabel.top + headingFontHeight;
#endif

	fProtocolsLabel = new BStringView(frameProtocolsLabel, "Protocols", _T("Protocols"));
	fProtocolsLabel->SetAlignment(B_ALIGN_LEFT);
	fProtocolsLabel->SetFont(&headingFont);

	BRect frameDivider2(frame);
#ifndef __HAIKU__
	frameDivider2.top = frameProtocolsLabel.bottom + inset;
	frameDivider2.bottom = frameDivider2.top + 25.0f;
#endif

	fProtocolDivider = new Divider(frameDivider2, "ProtocolDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fProtocolDivider->ResizeToPreferred();
	frameDivider2 = fProtocolDivider->Frame();

	BRect frameProtocolsDescLabel(frame);
#ifndef __HAIKU__
	frameProtocolsDescLabel.top = frameDivider2.bottom + inset;
	frameProtocolsDescLabel.bottom = frame.bottom;
#endif
	fProtocolsDesc = new MultiLineStringView(frameProtocolsDescLabel, "ProtocolsDesc", _T(kProtocolsDesc));
	fProtocolsDesc->MoveTo(frameProtocolsDescLabel.left, frameProtocolsDescLabel.top);
	fProtocolsDesc->ResizeToPreferred();

	fClientsLabel = new BStringView(frame, "ClientsLabel", _T("Clients"));
	fClientsLabel->SetAlignment(B_ALIGN_LEFT);
	fClientsLabel->SetFont(&headingFont);

	BStringView *clientsDescLabel = new BStringView(frame, NULL,
		_T("Clients provide the interface between you, the user, and the Server."));
	clientsDescLabel->SetAlignment(B_ALIGN_LEFT);

	BBox *divider3 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);

	fProtocolsButton = new BButton(frame, "protocols_edit", _T("Edit..."), new BMessage(kMsgEditProtocols));
	fClientsButton = new BButton(frame, "clients_edit", _T("Edit..."), new BMessage(kMsgEditClients));

#ifdef __HAIKU__
	fServerLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fServerDesc->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fServerDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fProtocolsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fProtocolsDesc->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fProtocolDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fClientsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	clientsDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	divider3->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(fServerLabel, 0, 0, 2)
			.Add(fServerDivider, 0, 1, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 2, 2)
			.Add(fServerDesc, 0, 3, 2)
			.Add(BSpaceLayoutItem::CreateHorizontalStrut(2.0f), 0, 4)
			.Add(fServerButton, 1, 4)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(8.0f), 0, 5, 2)

			.Add(fProtocolsLabel, 0, 6, 2)
			.Add(fProtocolDivider, 0, 7, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 8, 2)
			.Add(fProtocolsDesc, 0, 9, 2)
			.Add(BSpaceLayoutItem::CreateHorizontalStrut(2.0f), 0, 10)
			.Add(fProtocolsButton, 1, 11)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(8.0f), 0, 12, 2)

			.Add(fClientsLabel, 0, 13, 2)
			.Add(divider3, 0, 14, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 15, 2)
			.Add(clientsDescLabel, 0, 16, 2)
			.Add(BSpaceLayoutItem::CreateHorizontalStrut(2.0f), 0, 17)
			.Add(fClientsButton, 1, 17)
		)

		.AddGlue()
		.SetInsets(inset, inset, inset, inset)
	);
#else
	AddChild(fServerLabel);
	AddChild(fServerDivider);
	AddChild(fServerDesc);
	AddChild(fServerButton);
	AddChild(fProtocolsLabel);
	AddChild(fProtocolDivider);
	AddChild(fProtocolsDesc);
//	AddChild(fProtocolsButton);
//	AddChild(fClientsLabel);
//	AddChild(divider3);
//	AddChild(clientsDescLabel);
//	AddChild(fClientsButton);

	LayoutGUI();
#endif
}


void
PSettingsOverview::AttachedToWindow()
{
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif

	fServerButton->SetTarget(Parent()->Parent());
	fProtocolsButton->SetTarget(Parent()->Parent());
	fClientsButton->SetTarget(Parent()->Parent());
}

//#pragma mark Private

#ifndef __HAIKU__

void PSettingsOverview::LayoutGUI(void) {
};

#endif