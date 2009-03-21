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
#include "MultipleViewHandler.h"

#include <common/Divider.h>
#include <common/MultiLineStringView.h>

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

//#pragma mark Constants

const char *kServerDesc = "The Server is responsible for handling contact statuses, loading protocols and communicating between the protocols and clients.";
const char *kProtocolsDesc = "Protocols are responsible for sending and receiving messages, status updates and other information to Instant Messaging networks";
const char *kClientsDesc = "Clients are responsible for sending and receiving information to the Server. They provide functionality such as status notification, logging and sending and receiving messages."; 

const int32 kMsgEditServer = 'Mesr';
const int32 kMsgEditProtocols = 'Mpro';
const int32 kMsgEditClients = 'Mcli';

//#pragma mark Constructor

PSettingsOverview::PSettingsOverview(MultipleViewHandler *handler, BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fHandler(handler),
	fServerLabel(NULL),
	fServerDivider(NULL),
	fServerDesc(NULL),
	fServerButton(NULL),
	fProtocolsLabel(NULL),
	fProtocolsDivider(NULL),
	fProtocolsDesc(NULL),
	fProtocolsButton(NULL),
	fClientsLabel(NULL),
	fClientsDivider(NULL),
	fClientsDesc(NULL),
	fClientsButton(NULL)
{
#ifdef __HAIKU__
	BRect frame(0, 0, 1, 1);
#else
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	BRect frame;
	frame.InsetBy(inset * 2, inset * 2);
#endif
	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);

	// Server Details
	fServerLabel = new BStringView(frame, "ServerLabel", _T("Server"));
	fServerLabel->SetAlignment(B_ALIGN_LEFT);
	fServerLabel->SetFont(&headingFont);

	fServerDivider = new Divider(frame, "ServerDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fServerDivider->ResizeToPreferred();

	fServerDesc = new MultiLineStringView("ServersDesc", _T(kServerDesc), 200);
	//fServerDesc->ResizeToPreferred();

	fServerButton = new BButton(frame, "ServerEdit", _T("Edit" B_UTF8_ELLIPSIS), new BMessage(kMsgEditServer));
	fServerButton->ResizeToPreferred();

	// Protocol Details
	fProtocolsLabel = new BStringView(frame, "Protocols", _T("Protocols"));
	fProtocolsLabel->SetAlignment(B_ALIGN_LEFT);
	fProtocolsLabel->SetFont(&headingFont);

	fProtocolsDivider = new Divider(frame, "ProtocolDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fProtocolsDivider->ResizeToPreferred();

	fProtocolsDesc = new MultiLineStringView("ProtocolsDesc", _T(kProtocolsDesc), Bounds().Width());
	fProtocolsDesc->ResizeToPreferred();

	fProtocolsButton = new BButton(frame, "ProtocolEdit", _T("Edit" B_UTF8_ELLIPSIS), new BMessage(kMsgEditProtocols));
	fProtocolsButton->ResizeToPreferred();

	// Client Details
	fClientsLabel = new BStringView(frame, "ClientsLabel", _T("Clients"));
	fClientsLabel->SetAlignment(B_ALIGN_LEFT);
	fClientsLabel->SetFont(&headingFont);

	fClientsDivider = new Divider(frame, "ClientsDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fClientsDivider->ResizeToPreferred();
	
	fClientsDesc = new MultiLineStringView("ClientsDesc", _T(kClientsDesc), Bounds().Width());
	fClientsDesc->ResizeToPreferred();

	fClientsButton = new BButton(frame, "ClientsEdit", _T("Edit" B_UTF8_ELLIPSIS), new BMessage(kMsgEditClients));
	fClientsButton->ResizeToPreferred();

#ifdef __HAIKU__
	float width, height;
	fServerLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fServerDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fServerDesc->GetPreferredSize(&width, &height);
	fServerDesc->SetExplicitMaxSize(BSize(200, 100*height));
	fProtocolsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fProtocolsDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fProtocolsDesc->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fClientsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fClientsDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fClientsDesc->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, 0.0f)
		.Add(BGridLayoutBuilder(0.0f, 0.0f)
			.Add(fServerLabel, 0, 0, 2)
			.Add(fServerDivider, 0, 1, 2)
			.Add(fServerDesc, 0, 3, 2)
			.Add(fServerButton, 1, 4, 2)

			.Add(fProtocolsLabel, 0, 6, 2)
			.Add(fProtocolsDivider, 0, 7, 2)
			.Add(fProtocolsDesc, 0, 9, 2)
			.Add(fProtocolsButton, 1, 11)

			.Add(fClientsLabel, 0, 13, 2)
			.Add(fClientsDivider, 0, 14, 2)
			.Add(fClientsDesc, 0, 16, 2)
			.Add(fClientsButton, 1, 17)
		)

		.AddGlue()
	);
#else
	AddChild(fServerLabel);
	AddChild(fServerDivider);
	AddChild(fServerDesc);
	AddChild(fServerButton);
	AddChild(fProtocolsLabel);
	AddChild(fProtocolsDivider);
	AddChild(fProtocolsDesc);
	AddChild(fProtocolsButton);
	AddChild(fClientsLabel);
	AddChild(fClientsDivider);
	AddChild(fClientsDesc);
	AddChild(fClientsButton);

	LayoutGUI();
#endif
}

//#pragma mark BView Hooks

void
PSettingsOverview::AttachedToWindow(void)
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

	fServerButton->SetTarget(this);
	fProtocolsButton->SetTarget(this);
	fClientsButton->SetTarget(this);
};

void PSettingsOverview::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kMsgEditServer: {
			fHandler->ShowServerOverview();
		} break;
		
		case kMsgEditProtocols: {
			fHandler->ShowProtocolsOverview();
		} break;
		
		case kMsgEditClients: {
			fHandler->ShowClientsOverview();
		} break;
		
		default: {
			BView::MessageReceived(msg);
		} break;
	};
};

//#pragma mark Private

#ifndef __HAIKU__

void PSettingsOverview::LayoutGUI(void) {
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

	BRect frameServerDesc = fServerDesc->Frame();
	fServerDesc->MoveTo(frameServerDesc.left, frameServerDivider.bottom + inset);
	frameServerDesc = fServerDesc->Frame();

	BRect frameServerButton = fServerButton->Frame();
	fServerButton->MoveTo(frame.right - inset - frameServerButton.Width(), frameServerDesc.bottom + inset);
	frameServerButton = fServerButton->Frame();

	// Protocol related controls
	fProtocolsLabel->ResizeToPreferred();
	BRect frameProtocolsLabel = fProtocolsLabel->Frame();
	fProtocolsLabel->MoveTo(frameProtocolsLabel.left, frameServerButton.bottom + inset);
	frameProtocolsLabel = fProtocolsLabel->Frame();
	
	BRect frameProtocolsDivider = fProtocolsDivider->Frame();
	fProtocolsDivider->MoveTo(frameProtocolsDivider.left, frameProtocolsLabel.bottom + inset);
	frameProtocolsDivider = fProtocolsDivider->Frame();
	
	BRect frameProtocolsDesc = fProtocolsDesc->Frame();
	fProtocolsDesc->MoveTo(frameProtocolsDesc.left, frameProtocolsDivider.bottom + inset);
	frameProtocolsDesc = fProtocolsDesc->Frame();

	BRect frameProtocolsButton = fProtocolsButton->Frame();
	fProtocolsButton->MoveTo(frame.right - inset - frameProtocolsButton.Width(), frameProtocolsDesc.bottom + inset);
	frameProtocolsButton = fProtocolsButton->Frame();
	
	// Client related controls
	fClientsLabel->ResizeToPreferred();
	BRect frameClientsLabel = fClientsLabel->Frame();
	fClientsLabel->MoveTo(frameClientsLabel.left, frameProtocolsButton.bottom + inset);
	frameClientsLabel = fClientsLabel->Frame();

	BRect frameClientsDivider = fClientsDivider->Frame();
	fClientsDivider->MoveTo(frameClientsDivider.left, frameClientsLabel.bottom + inset);
	frameClientsDivider = fClientsDivider->Frame();
	
	BRect frameClientsDesc = fClientsDesc->Frame();
	fClientsDesc->MoveTo(frameClientsDesc.left, frameClientsDivider.bottom + inset);
	frameClientsDesc = fClientsDesc->Frame();
	
	BRect frameClientsButton = fClientsButton->Frame();
	fClientsButton->MoveTo(frame.right - inset - frameClientsButton.Width(), frameClientsDesc.bottom + inset);
	frameClientsButton = fClientsButton->Frame();
};

#endif
