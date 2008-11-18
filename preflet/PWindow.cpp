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
#endif

#include "PWindow.h"
#include "PView.h"

#ifdef ZETA
#	include <app/Roster.h>
#	include <locale/Locale.h>
#	include <storage/Path.h>
#else
#	define _T(str) (str)
#endif

#ifndef B_AUTO_UPDATE_SIZE_LIMITS
#	define B_AUTO_UPDATE_SIZE_LIMITS 0
#endif

#ifndef B_CLOSE_ON_ESCAPE
#	define B_CLOSE_ON_ESCAPE 0
#endif

PWindow::PWindow()
	: BWindow(BRect(0, 0, 720, 340), _T("Instant Messaging"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
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

#ifdef __HAIKU__
	// Build layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));
#endif

	// Add top view to the layout
	fView = new PView(Bounds());
#ifdef __HAIKU__
	GetLayout()->AddView(fView);
#else
	AddChild(fView);
#endif

	CenterWindowOnScreen();
}


bool
PWindow::QuitRequested()
{
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	return true;
}


void PWindow::CenterWindowOnScreen()
{
	BRect screenFrame = BScreen().Frame();
	BPoint pt;

	pt.x = screenFrame.Width()/2 - Bounds().Width()/2;
	pt.y = screenFrame.Height()/2 - Bounds().Height()/2;

	if (screenFrame.Contains(pt))
		MoveTo(pt);
}
