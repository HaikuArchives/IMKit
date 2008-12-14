/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif
#include <interface/TextControl.h>
#include <interface/Button.h>
#include <storage/Path.h>

#include "PAccountDialog.h"
#include "PUtils.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

#ifndef B_AUTO_UPDATE_SIZE_LIMITS
#	define B_AUTO_UPDATE_SIZE_LIMITS 0
#endif

#ifndef B_CLOSE_ON_ESCAPE
#	define B_CLOSE_ON_ESCAPE 0
#endif

const int32 kAddAccountCancel = 'Mcnl';
const int32 kAddAccountOk = 'Mokb';

const int32 kSemTimeOut = 50000;

PAccountDialog::PAccountDialog(const char* title, BPath* addonPath)
	: BWindow(BRect(0, 0, 320, 75), title, B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fSem(-1),
	fValue(-1)
{
	BRect frame(0, 0, 1, 1);
	BView* view = new BView(frame, "top", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

#ifdef __HAIKU__
	fAccountName = new BTextControl("account_name", _T("Account name:"), NULL, NULL);
#else
	fAccountName = new BTextControl(frame, "account_name", _T("Account name:"), NULL, NULL);
#endif
	fAccountName->MakeFocus();

	BButton* cancelButton = new BButton(frame, "cancel", _T("Cancel"), new BMessage(kAddAccountCancel));
	BButton* okButton = new BButton(frame, "ok", _T("OK"), new BMessage(kAddAccountOk));

#ifdef __HAIKU__
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	view->SetLayout(new BGroupLayout(B_VERTICAL));
	view->AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.Add(fAccountName->CreateLabelLayoutItem())
			.Add(fAccountName->CreateTextViewLayoutItem())
		.End()

		.AddGlue()

		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancelButton)
			.Add(okButton)
		.End()

		.SetInsets(inset, inset, inset, inset)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	GetLayout()->AddView(view);
#else
	view->AddChild(fAccountName);
	view->AddChild(cancelButton);
	view->AddChild(okButton);
	window->AddChild(view);
#endif

	CenterWindowOnScreen(this);
}


void
PAccountDialog::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kAddAccountOk:
		case kAddAccountCancel:
			if (fSem < B_OK)
				PostMessage(B_QUIT_REQUESTED);
			else {
				if (msg->what == kAddAccountOk)
					fValue = 1;
				else
					fValue = 0;
				delete_sem(fSem);
				fSem = -1;
			}
			break;

		default:
			BWindow::MessageReceived(msg);
			break;
	}
}


int32
PAccountDialog::Go()
{
	fSem = create_sem(0, "AccountsDialogSem");
	if (fSem < B_OK) {
		Quit();
		return -1;
	}

	// Get the originating window, if exists
	BWindow* window =
		dynamic_cast<BWindow*>(BLooper::LooperForThread(find_thread(NULL)));

	Show();

	if (window) {
		status_t err;
		for (;;) {
			do {
				err = acquire_sem_etc(fSem, 1, B_RELATIVE_TIMEOUT, kSemTimeOut);
			} while (err == B_INTERRUPTED);

			if (err == B_BAD_SEM_ID) {
				// Semaphore gone
				break;
			}
			window->UpdateIfNeeded();
		}
	} else {
		// No window to update, so just hang out until we're done
		while (acquire_sem(fSem) == B_INTERRUPTED);
	}

	int32 value = fValue;
	if (Lock())
		Quit();

	return value;
}


const char*
PAccountDialog::AccountName()
{
	if (fValue == 1)
		return fAccountName->Text();
	return NULL;
}
