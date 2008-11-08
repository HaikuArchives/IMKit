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
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif


PWindow::PWindow()
	: BWindow(BRect(0, 0, 520, 320), _T("Instant Messaging"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
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
	PView* top = new PView();
#ifdef __HAIKU__
	GetLayout()->AddView(top);
#else
	AddChild(top);
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
