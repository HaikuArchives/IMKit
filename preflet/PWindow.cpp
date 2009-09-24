/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		Mikael Eiman <m_eiman@eiman.tv>
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <app/Application.h>
#include <interface/Screen.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif

#include "PWindow.h"
#include "PView.h"
#include "PUtils.h"

#ifdef ZETA
#	include <app/Roster.h>
#	include <locale/Locale.h>
#	include <storage/Path.h>
#else
#	define _T(str) (str)
#endif

#ifndef __HAIKU__
#	define B_AUTO_UPDATE_SIZE_LIMITS 0
#	define B_CLOSE_ON_ESCAPE 0
#endif

#ifdef __HAIKU__
#	define PWINDOW_RECT 0, 0, 1, 1
#else
#	define PWINDOW_RECT 0, 0, 720, 500
#endif

//#pragma mark Constructors

PWindow::PWindow(void)
	: BWindow(BRect(PWINDOW_RECT), _T("Instant Messaging"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE) {
#ifdef ZETA
	app_info ai;
	be_app->GetAppInfo(&ai);

	BPath path;
	BEntry entry(&ai.ref, true);

	entry.GetPath(&path);
	path.GetParent(&path);
	path.Append("Language/Dictionaries/InstantMessaging");

	BString path_string;

	if (path.InitCheck() != B_OK)
		path_string.SetTo("Language/Dictionaries/InstantMessaging");
	else
		path_string.SetTo(path.Path());

	be_locale.LoadLanguageFile(path_string.String());
#endif

	// Add top view to the layout
	fView = new PView(Bounds());

#ifdef __HAIKU__
	// Build layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fView)
	);
#else
	AddChild(fView);
#endif

#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(0, 0, 0, 0);
#endif

	CenterWindowOnScreen(this);
}

//#pragma mark BWindow Hooks

bool PWindow::QuitRequested(void) {
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	return true;
}
