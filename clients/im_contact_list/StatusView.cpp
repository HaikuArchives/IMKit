/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Message.h>
#include <GroupLayout.h>
#include <GridLayoutBuilder.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <MenuField.h>

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>

#include "StatusView.h"
#include "PictureView.h"
#include "Misc.h"

const char* kLogName = "im_contact_list";

const int32 kSetStatus = 'sets';


StatusView::StatusView(const char* name)
	: BView(name, B_WILL_DRAW),
	fStatus(IM::Offline)
{
	// PopUp menu
	fStatusMenu = new BPopUpMenu("Status menu");

	// Online status
	BMessage* setOnline = new BMessage(kSetStatus);
	setOnline->AddString("status", ONLINE_TEXT);
	fOnlineMenuItem = new BMenuItem(ONLINE_TEXT, setOnline);
	fStatusMenu->AddItem(fOnlineMenuItem);

	BMessage* setAway = new BMessage(kSetStatus);
	setAway->AddString("status", AWAY_TEXT);
	fAwayMenuItem = new BMenuItem(AWAY_TEXT, setAway);
	fStatusMenu->AddItem(fAwayMenuItem);

	// Offline
	BMessage* setOffline = new BMessage(kSetStatus);
	setOffline->AddString("status", OFFLINE_TEXT);
	fOfflineMenuItem = new BMenuItem(OFFLINE_TEXT, setOffline);
	fStatusMenu->AddItem(fOfflineMenuItem);

	// Menu field
	fStatusMenuField = new BMenuField("status_field", NULL, fStatusMenu, NULL);

	// Avatar icon
	PictureView* avatar = new PictureView("");

	// Layout
	float inset = (float)ceilf(be_plain_font->Size() * 0.7f);
	SetLayout(new BGroupLayout(B_VERTICAL, inset));
	AddChild(BGridLayoutBuilder(inset, inset)
		.Add(fStatusMenuField, 0, 0)
		.Add(avatar, 1, 0)
	);

	// Update user interface
	GetProtocolStatuses();
}


void
StatusView::AttachedToWindow()
{
	fStatusMenu->SetTargetForItems(this);
}


void
StatusView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kSetStatus: {
				const char* status = NULL;

				if (msg->FindString("status", &status) != B_OK) {
					LOG(kLogName, liDebug, "No 'status' field in kSetStatus message");
					return;
				}

				BMessage cmd(IM::MESSAGE);
				cmd.AddInt32("im_what", IM::SET_STATUS);
				cmd.AddString("status", status);
				(void)SendMessageToServer(&cmd);
			} break;
		default:
			BView::MessageReceived(msg);
			break;
	};
}


void
StatusView::GetProtocolStatuses()
{
	IM::Manager manager;
	BMessage statuses;

	if (manager.SendMessage(new BMessage(IM::GET_OWN_STATUSES), &statuses) != B_OK) {
		LOG(kLogName, liHigh, "Error getting protocol status");
		return;
	}

	BString instanceID;

	for (int32 i = 0; statuses.FindString("instance_id", i, &instanceID) == B_OK; i++) {
		const char* protocol = NULL;
		const char* account = NULL;
		const char* userFriendly = NULL;
		const char* status = NULL;

		if (statuses.FindString("protocol", i, &protocol) != B_OK)
			continue;
		if (statuses.FindString("account_name", i, &account) != B_OK)
			continue;
		if (statuses.FindString("userfriendly", i, &userFriendly) != B_OK)
			continue;
		if (statuses.FindString("status", i, &status) != B_OK)
			continue;

		IM::AccountInfo* info = new IM::AccountInfo(instanceID.String(), protocol,
			account, userFriendly, status);

		if ((fStatus > IM::Online) && (info->Status() == IM::Online))
			fStatus = IM::Online;
		if ((fStatus > IM::Away) && (info->Status() == IM::Away))
			fStatus = IM::Away;
	}

	switch (fStatus) {
		case IM::Online:
			fOnlineMenuItem->SetMarked(true);
			break;
		case IM::Away:
			fAwayMenuItem->SetMarked(true);
			break;
		case IM::Offline:
			fOfflineMenuItem->SetMarked(true);
			break;
	}
}
