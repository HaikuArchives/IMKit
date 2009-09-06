/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Alert.h>

#include "ContactListApp.h"
#include "ContactListResources.h"
#include "ContactListWindow.h"


ContactListApp::ContactListApp()
	: BApplication(CONTACT_LIST_SIGNATURE)
{
	ContactListWindow* win = new ContactListWindow();

	// Check if initialization is OK
	if (win->InitCheck() != B_OK) {
		// Display an error message
		BAlert* alert = new BAlert("IM Kit",
			"It seems the IM Kit server is not running, hence Contat List " \
			"can't work correctly.\nPlease, start IM Kit server from the Deskbar " \
			"icon before.", "OK", NULL, NULL,
			B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
		alert->Go();

		// Quit this application
		QuitRequested();
	}

	win->Show();
}
