/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <app/Message.h>
#include <interface/GroupLayout.h>
#include <interface/GridLayoutBuilder.h>
#include <interface/PopUpMenu.h>
#include <interface/MenuItem.h>
#include <interface/MenuField.h>

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>

#include "StatusView.h"
#include "PictureView.h"

const char* kLogName = "im_contact_list";

const int32 kSetOnline = '_ava';
const int32 kSetAway = '_awa';
const int32 kSetOffline = '_off';


StatusView::StatusView(const char* name)
	: BView(name, B_WILL_DRAW),
	fStatus(IM::Offline)
{
	// PopUp menu
	fStatusMenu = new BPopUpMenu("Status menu");
	fOnlineMenuItem = new BMenuItem("Online", new BMessage(kSetOnline));
	fAwayMenuItem = new BMenuItem("Away", new BMessage(kSetAway));
	fOfflineMenuItem = new BMenuItem("Offline", new BMessage(kSetOffline));
	fStatusMenu->AddItem(fOnlineMenuItem);
	fStatusMenu->AddItem(fAwayMenuItem);
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
