/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include "ContactListApp.h"
#include "ContactListResources.h"
#include "ContactListWindow.h"


ContactListApp::ContactListApp()
	: BApplication(CONTACT_LIST_SIGNATURE)
{
	ContactListWindow* win = new ContactListWindow();
	win->Show();
}
