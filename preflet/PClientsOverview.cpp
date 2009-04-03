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
#include "SettingsHost.h"

#include "common/Divider.h"
#include "common/MultiLineStringView.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

//#pragma mark ClientInfo

class ClientInfo {
	public:
					ClientInfo(BCheckBox *check, BButton *button)
						: fCheckBox(check),
						fButton(button) {
					};
		
		BCheckBox	*CheckBox(void) {
						return fCheckBox;
					};
		BButton		*Button(void) {
						return fButton;
					};
	
	private:
		BCheckBox	*fCheckBox;
		BButton		*fButton;
};

//#pragma mark Constants

const int32 kMsgEditClient = 'Mecl';
const int32 kMsgCheckChanged = 'Mcc_';
const char *kAutoStartDesc = "Clients set to autostart will start when the Server starts. This is useful for clients you will always use, such as notifications or chat windows.";

//#pragma mark Constructor

PClientsOverview::PClientsOverview(MultipleViewHandler *handler, BRect bounds)
	: BView(bounds, "clients", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fHandler(handler) {
	
	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Frame();
	frame.InsetBy(inset * 2, inset * 2);
#endif

	fAutoStartLabel = new BStringView(frame, "AutoStartLabel", _T("Autostart"));
	fAutoStartLabel->SetAlignment(B_ALIGN_LEFT);
	fAutoStartLabel->SetFont(&headingFont);

	fAutoStartDivider = new Divider(frame, "AutoStartDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fAutoStartDivider->ResizeToPreferred();

	fAutoStartDesc = new MultiLineStringView("AutoStartDesc", _T(kAutoStartDesc), Bounds().Width());
	fAutoStartDesc->ResizeToPreferred();

#ifdef __HAIKU__
	int32 row = 5;
	BGridLayoutBuilder layout(0.0f, 1.0f);
	layout.Add(fAutoStartLabel, 0, 0, 2)
		.Add(fAutoStartDivider, 0, 1, 2)
		.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 2, 2)
		.Add(fAutoStartDesc, 0, 3, 2)
		.Add(BSpaceLayoutItem::CreateVerticalStrut(4.0f), 0, 4, 2)
	;
#endif

	BMessage clients, msg;
	im_get_client_list(&clients);
	for (int32 i = 0; clients.FindMessage("client", i, &msg) == B_OK; i++) {
		const char *path = NULL;
		const char *file = NULL;

		// Get client path and file
		if (msg.FindString("path", &path) != B_OK) continue;
		if (msg.FindString("file", &file) != B_OK) continue;

		BCheckBox *checkbox = new BCheckBox(frame, path, file, new BMessage(kMsgCheckChanged));
		checkbox->SetValue(B_CONTROL_ON);
		checkbox->ResizeToPreferred();

		// Edit button
		BMessage* editMsg = new BMessage(kMsgEditClient);
		editMsg->AddString("path", path);
		editMsg->AddString("file", file);

		BString nameEdit = "edit_";
		nameEdit << file;
		BButton* button = new BButton(frame, nameEdit.String(), _T("Edit..."), editMsg);
		button->ResizeToPreferred();

		fClientInfo.push_back(new ClientInfo(checkbox, button));

#ifdef __HAIKU__
		layout.Add(checkbox, 0, row);
		layout.Add(button, 1, row);
		layout.Add(BSpaceLayoutItem::CreateVerticalStrut(8.0f), 0, ++row, 2);
#else
		AddChild(checkbox);
		AddChild(button);
#endif

		
	}

#ifdef __HAIKU__
	fAutoStartLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fAutoStartDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fAutoStartDesc->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(layout)
		.AddGlue()
		.SetInsets(inset, inset, inset, inset)
	);
#else
	AddChild(fAutoStartLabel);
	AddChild(fAutoStartDivider);
	AddChild(fAutoStartDesc);
	
	LayoutGUI();
#endif
}

//#pragma mark BView Hooks

void PClientsOverview::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif

	for (clientinfo_t::iterator cIt = fClientInfo.begin(); cIt != fClientInfo.end(); cIt++ ){
		ClientInfo *info = (*cIt);
		BCheckBox *checkbox = info->CheckBox();

		checkbox->SetTarget(this);		
	};
};

void PClientsOverview::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kMsgCheckChanged: {
			fHost->ControllerModified(this);
		} break;
		default: {
			BView::MessageReceived(msg);
		} break;
	};
};

//#pragma mark SettingsController Hooks

status_t PClientsOverview::Init(SettingsHost *host) {
	fHost = host;

	return B_OK;
};

status_t PClientsOverview::Save(const BMessage *tmplate, BMessage *settings) {
	return SettingsController::Save(tmplate, settings);
};

status_t PClientsOverview::Revert(const BMessage *tmplate) {
	return SettingsController::Revert(tmplate);
};


//#pragma mark Private

#ifndef __HAIKU__

void PClientsOverview::LayoutGUI(void) {
	font_height fh;
	BFont headingFont(be_bold_font);
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BRect frame = Bounds();
	frame.InsetBy(inset * 2, inset * 2);
	frame.OffsetBy(inset, inset);
	
	// Autostart
	fAutoStartLabel->ResizeToPreferred();
	BRect frameAutoStartLabel = fAutoStartLabel->Frame();
	
	BRect frameAutoStartDivider = fAutoStartDivider->Frame();
	fAutoStartDivider->MoveTo(frameAutoStartDivider.left, frameAutoStartLabel.bottom + inset);
	frameAutoStartDivider = fAutoStartDivider->Frame();
	
	BRect frameAutoStartDesc = fAutoStartDesc->Frame();
	fAutoStartDesc->MoveTo(frameAutoStartDesc.left, frameAutoStartDivider.bottom + inset);
	frameAutoStartDesc = fAutoStartDesc->Frame();
	
	BRect previous = frameAutoStartDesc;
	
	for (clientinfo_t::iterator cIt = fClientInfo.begin(); cIt != fClientInfo.end(); cIt++ ){
		ClientInfo *info = (*cIt);
		BCheckBox *checkbox = info->CheckBox();
		BButton *button = info->Button();
		
		BRect frameCheckBox = checkbox->Frame();
		BRect frameButton = button->Frame();
		
		checkbox->MoveTo(frameCheckBox.left, previous.bottom + inset);
		button->MoveTo(frame.right - inset - frameButton.Width(), previous.bottom + inset);
		
		previous = button->Frame();
	};
};

#endif
