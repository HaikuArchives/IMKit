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

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

PSettingsOverview::PSettingsOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect frame(0, 0, 1, 1);
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BStringView* serversLabel = new BStringView(frame, NULL, _T("Servers"));
	serversLabel->SetAlignment(B_ALIGN_LEFT);
	serversLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	serversLabel->SetFont(be_bold_font);

	BStringView* serversDescLabel = new BStringView(frame, NULL,
		_T("The server is responsible for the IM Kit activity."));
	serversDescLabel->SetAlignment(B_ALIGN_LEFT);
	serversDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BBox* divider1 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider1->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	BStringView* protocolsLabel = new BStringView(frame, NULL, _T("Protocols"));
	protocolsLabel->SetAlignment(B_ALIGN_LEFT);
	protocolsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	protocolsLabel->SetFont(be_bold_font);

	BStringView* protocolsDescLabel = new BStringView(frame, NULL,
		_T("Protocols communicate with instant messaging networks."));
	protocolsDescLabel->SetAlignment(B_ALIGN_LEFT);
	protocolsDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BBox* divider2 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider2->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	BStringView* clientsLabel = new BStringView(frame, NULL, _T("Clients"));
	clientsLabel->SetAlignment(B_ALIGN_LEFT);
	clientsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	clientsLabel->SetFont(be_bold_font);

	BStringView* clientsDescLabel = new BStringView(frame, NULL,
		_T("Clients provide the interface between you, the user, and the Server."));
	clientsDescLabel->SetAlignment(B_ALIGN_LEFT);
	clientsDescLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BBox* divider3 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider3->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	fServersButton = new BButton(frame, "servers_edit", _T("Edit..."), new BMessage(kMsgEditServers));
	fProtocolsButton = new BButton(frame, "protocols_edit", _T("Edit..."), new BMessage(kMsgEditProtocols));
	fClientsButton = new BButton(frame, "clients_edit", _T("Edit..."), new BMessage(kMsgEditClients));

#ifdef __HAIKU__
	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(serversLabel, 0, 0, 2)
			.Add(divider1, 0, 1, 2)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 2, 2)
			.Add(serversDescLabel, 0, 3, 2)
			.Add(BSpaceLayoutItem::CreateHorizontalStrut(2.0f), 0, 4)
			.Add(fServersButton, 1, 4)
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
	AddChild(serversLabel);
#endif
}


void
PSettingsOverview::AttachedToWindow()
{
	fServersButton->SetTarget(Parent()->Parent());
	fProtocolsButton->SetTarget(Parent()->Parent());
	fClientsButton->SetTarget(Parent()->Parent());
}
