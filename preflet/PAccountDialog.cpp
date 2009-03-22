/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/SpaceLayoutItem.h>
#endif

#include <app/Message.h>
#include <app/Messenger.h>
#include <interface/Box.h>
#include <interface/Button.h>
#include <interface/TextControl.h>
#include <interface/View.h>
#include <storage/Path.h>

#include <libim/Helpers.h>

#include "common/Divider.h"

#include "PAccountDialog.h"
#include "PUtils.h"
#include "ViewFactory.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

#ifndef B_AUTO_UPDATE_SIZE_LIMITS
#	define B_AUTO_UPDATE_SIZE_LIMITS 0
#endif

#ifndef B_SUPPORTS_LAYOUT
#	define B_SUPPORTS_LAYOUT 0
#endif

#ifndef B_CLOSE_ON_ESCAPE
#	define B_CLOSE_ON_ESCAPE 0
#endif

//#pragma mark Constants

const uint32 kAddAccountCancel = 'Mcnl';
const uint32 kAddAccountOk = 'Mokb';

//#pragma mark Constructor

PAccountDialog::PAccountDialog(const char *title, const char *protocol, const char *account, BMessage settingsTemplate, BMessage settings, BMessenger *target, BMessage save, BMessage cancel)
	: BWindow(BRect(0, 0, 360, 400), title, B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fOriginalAccount(account),
	fTemplate(settingsTemplate),
	fSettings(settings),
	fTarget(target),
	fSave(save),
	fCancel(cancel) {
	
	uint32 childResizeMode = B_FOLLOW_NONE;
	BRect frame(0, 0, 1, 1);

	BView* view = new BView(frame, "top", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

#ifndef __HAIKU__
	childResizeMode = B_FOLLOW_ALL_SIDES;
#endif

	// Account name
	fAccountName = ViewFactory::Create<BTextControl>(frame, "account_name", childResizeMode, B_WILL_DRAW | B_FRAME_EVENTS);
	fAccountName->SetLabel(_T("Account name:"));
	fAccountName->SetText(fOriginalAccount.String());
	fAccountName->SetFont(be_bold_font);
	fAccountName->MakeFocus();

	// Account name divider
	fAccountNameDivider = new Divider(frame, "AccountNameDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fAccountNameDivider->ResizeToPreferred();

	fProtocolControl = new BView(frame, "ProtocolControls", B_FOLLOW_ALL_SIDES, B_NAVIGABLE_JUMP | B_WILL_DRAW);
	BuildGUI(fTemplate, fSettings, protocol, fProtocolControl, false);

	BButton* cancelButton = new BButton(frame, "cancel", _T("Cancel"), new BMessage(kAddAccountCancel));
	BButton* okButton = new BButton(frame, "ok", _T("OK"), new BMessage(kAddAccountOk));

#ifdef __HAIKU__
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	fAccountNameDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fProtocolControl->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	view->SetLayout(new BGroupLayout(B_VERTICAL));
	view->AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.AddGroup(B_VERTICAL, inset)
			.Add(fAccountName)
			.Add(fAccountNameDivider)
			.Add(fProtocolControl)
		.End()
		.SetInsets(inset, inset, inset, inset)
	);
	view->AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.AddGroup(B_HORIZONTAL, inset)
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
	AddChild(view);
#endif

	CenterWindowOnScreen(this);
};

//#pragma mark BWindow Hooks

void PAccountDialog::MessageReceived(BMessage* msg) {
	switch (msg->what) {
		case kAddAccountOk:
		case kAddAccountCancel: {
			bool save = (msg->what == kAddAccountOk);
			
			if (save == true) {
				BMessage settings;
				SaveSettings(fProtocolControl, fTemplate, &settings);
				
				fSave.AddMessage("settings", &settings);
				fSave.AddString("name", fAccountName->Text());
				fSave.AddString("original_name", fOriginalAccount);
			};

			SendNotification(save);
		} break;

		default: {
			BWindow::MessageReceived(msg);
		} break;
	};
};

bool PAccountDialog::QuitRequested(void) {
	SendNotification(false);
	
	return BWindow::QuitRequested();
}

//#pragma mark Public

const char *PAccountDialog::AccountName(void) {
	return fAccountName->Text();
};

//#pragma mark Private

void PAccountDialog::SendNotification(bool saved) {		
	if (fTarget != NULL) {
		BMessage targetMsg = fSave;
		BMessage other = fCancel;
		
		if (saved == false) {
			targetMsg = fCancel;
			other = fSave;
		}

		if (targetMsg.what != 0) {
			targetMsg.AddPointer("source", this);
			fTarget->SendMessage(&targetMsg);
		}
		
		targetMsg.PrintToStream();
		
		delete fTarget;
		
		fSave.MakeEmpty();
		fCancel.MakeEmpty();
		fTarget = NULL;
	};
};
