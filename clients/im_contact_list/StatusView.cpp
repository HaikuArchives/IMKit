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

#include "StatusView.h"
#include "PictureView.h"

const int32 kAvailable = '_ava';
const int32 kAway = '_awa';
const int32 kOffline = '_off';


StatusView::StatusView(const char* name)
	: BView(name, B_WILL_DRAW)
{
	// PopUp menu
	fStatusMenu = new BPopUpMenu("Status menu");
	fStatusMenu->AddItem(new BMenuItem("Available", new BMessage(kAvailable)));
	fStatusMenu->AddItem(new BMenuItem("Away", new BMessage(kAway)));
	fStatusMenu->AddItem(new BMenuItem("Offline", new BMessage(kOffline)));

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
}


void
StatusView::AttachedToWindow()
{
	fStatusMenu->SetTargetForItems(this);
}
