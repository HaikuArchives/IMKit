/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <app/Application.h>
#include <interface/GroupLayout.h>
#include <interface/GroupLayoutBuilder.h>

#include "ContactListWindow.h"
#include "PeopleColumnListView.h"
#include "StatusView.h"


ContactListWindow::ContactListWindow()
	: BWindow(BRect(0, 0, 1, 1), "Contact List", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
	// Views
	fListView = new PeopleColumnListView("contact_list");
	fStatusView = new StatusView("status");

	// Populate list view
	fListView->Populate();

	// Layout
	float inset = (float)ceilf(be_plain_font->Size() * 0.7f);
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(fListView)
		.Add(fStatusView)
	);

	// Center window
	CenterOnScreen();
}


bool
ContactListWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
