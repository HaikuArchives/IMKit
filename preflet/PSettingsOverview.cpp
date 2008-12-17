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

//#pragma mark Constructor

PSettingsOverview::PSettingsOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect frame(0, 0, 1, 1);
	
#ifndef __HAIKU__
	frame = Bounds();
#endif

	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);

	font_height fh;
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;

	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BRect frameServerLabel(frame);
#ifndef __HAIKU__
	frameServerLabel.bottom = headingFontHeight;
#endif

	BStringView* serverLabel = new BStringView(frameServerLabel, NULL, _T("Server"));
	serverLabel->SetAlignment(B_ALIGN_LEFT);
	serverLabel->SetFont(&headingFont);

	BRect frameDivider1(frame);
#ifndef __HAIKU__
	frameDivider1.top = frameServerLabel.bottom + inset;
//	frameDivider1.bottom = frameDivider1.top + 5.0f;
//	frameDivider1.bottom = frameDivider1.top + 25.0f;
frameDivider1.bottom = 100;
#endif

	Divider* divider1 = new Divider(frameDivider1, "divider1", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);

	BRect frameServerDescLabel(frame);
#ifndef __HAIKU__
	frameServerDescLabel.top = frameDivider1.bottom + inset;
	frameServerDescLabel.bottom = frame.bottom;
#endif

	MultiLineStringView* serverDescLabel = new MultiLineStringView(NULL, _T(kServerDesc), frameServerDescLabel.Width());
	serverDescLabel->MoveTo(frameServerDescLabel.left, frameServerDescLabel.top);
	serverDescLabel->ResizeToPreferred();
	
	BRect frameServerButton(frame);
#ifndef __HAIKU__
	frameServerButton.top = serverDescLabel->Frame().bottom + inset;
	frameServerButton.bottom = frameServerButton.top + 20;
#endif

	fServerButton = new BButton(frameServerButton, "server_edit", _T("Edit..."), new BMessage(kMsgEditServer));
#ifndef __HAIKU__
	fServerButton->ResizeToPreferred();
	BRect frameServerPreferred = fServerButton->Frame();
	fServerButton->MoveTo(frame.right - inset - frameServerPreferred.Width(), frameServerButton.top);
#endif

	BStringView* protocolsLabel = new BStringView(frame, NULL, _T("Protocols"));
	protocolsLabel->SetAlignment(B_ALIGN_LEFT);
	protocolsLabel->SetFont(&headingFont);

	BStringView* protocolsDescLabel = new BStringView(frame, NULL,
		_T("Protocols communicate with instant messaging networks."));
	protocolsDescLabel->SetAlignment(B_ALIGN_LEFT);

	BBox* divider2 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);

	BStringView* clientsLabel = new BStringView(frame, NULL, _T("Clients"));
	clientsLabel->SetAlignment(B_ALIGN_LEFT);
	clientsLabel->SetFont(&headingFont);

	BStringView* clientsDescLabel = new BStringView(frame, NULL,
		_T("Clients provide the interface between you, the user, and the Server."));
	clientsDescLabel->SetAlignment(B_ALIGN_LEFT);

	BBox* divider3 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);

	fProtocolsButton = new BButton(frame, "protocols_edit", _T("Edit..."), new BMessage(kMsgEditProtocols));
	fClientsButton = new BButton(frame, "clients_edit", _T("Edit..."), new BMessage(kMsgEditClients));

#ifdef __HAIKU__
	serverLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	serverDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	divider1->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	protocolsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	protocolsDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	divider2->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	clientsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	clientsDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	divider3->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(serverLabel, 0, 0, 2)
			.Add(divider1, 0, 1, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 2, 2)
			.Add(serverDescLabel, 0, 3, 2)
			.Add(BSpaceLayoutItem::CreateHorizontalStrut(2.0f), 0, 4)
			.Add(fServerButton, 1, 4)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(8.0f), 0, 5, 2)

			.Add(protocolsLabel, 0, 6, 2)
			.Add(divider2, 0, 7, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 8, 2)
			.Add(protocolsDescLabel, 0, 9, 2)
			.Add(BSpaceLayoutItem::CreateHorizontalStrut(2.0f), 0, 10)
			.Add(fProtocolsButton, 1, 11)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(8.0f), 0, 12, 2)

			.Add(clientsLabel, 0, 13, 2)
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
	AddChild(serverLabel);
	AddChild(divider1);
	AddChild(serverDescLabel);
	AddChild(fServerButton);
	AddChild(divider2);
	AddChild(protocolsDescLabel);
//	AddChild(fProtocolsButton);
//	AddChild(clientsLabel);
//	AddChild(divider3);
//	AddChild(clientsDescLabel);
//	AddChild(fClientsButton);
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
