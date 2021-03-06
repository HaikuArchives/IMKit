/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Application.h>

#ifdef __HAIKU__
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#endif

#include "ContactListWindow.h"
#include "ContactListView.h"


ContactListWindow::ContactListWindow()
	: BWindow(BRect(0, 0, 1, 1), "Contact List", B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS
#ifdef __HAIKU__
		| B_AUTO_UPDATE_SIZE_LIMITS
#endif
		),
	fManager(new IM::Manager(BMessenger(this)))
{
	// Create contact list view
	fView = new ContactListView("top");

#ifdef __HAIKU__
	// Layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fView)
	);

	// Center window
	CenterOnScreen();
#endif
}


status_t
ContactListWindow::InitCheck()
{
	return fManager->InitCheck();
}


bool
ContactListWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
