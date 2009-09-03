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
#include "PClientView.h"
#include "PUtils.h"
#include "ViewFactory.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

#ifdef __HAIKU__
//#	define WINDOW_RECT BRect(0, 0, 1, 1)
#	define WINDOW_RECT BRect(0, 0, 360, 400)
#else
#	define B_AUTO_UPDATE_SIZE_LIMITS 0
#	define B_CLOSE_ON_ESCAPE 0
#	define WINDOW_RECT BRect(0, 0, 360, 400)
#endif

//#pragma mark Constants

const uint32 kAddAccountCancel = 'Mcnl';
const uint32 kAddAccountOk = 'Mokb';

//#pragma mark Constructor

PAccountDialog::PAccountDialog(const char *title, const char *protocol,
                               const char *account, BMessage settingsTemplate,
                               BMessage settings, BMessenger *target,
                               BMessage save, BMessage cancel)
	: BWindow(WINDOW_RECT, title, B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS |
                B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fOriginalAccount(account),
	fTemplate(settingsTemplate),
	fSettings(settings),
	fTarget(target),
	fSave(save),
	fCancel(cancel)
{
	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Bounds();
#endif

	BView *view = ViewFactory::Create<BView>(frame, "top", B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS);

	// Account name
	fAccountName = ViewFactory::Create<BTextControl>(frame, "account_name", _T("Account name:"),
		fOriginalAccount.String(), NULL, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fAccountName->SetFont(be_bold_font);
	fAccountName->MakeFocus();

	// Account name divider
	fAccountNameDivider = new Divider(frame, "AccountNameDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fAccountNameDivider->ResizeToPreferred();

	fProtocolControl = new PClientView(frame, "ProtocolControls", NULL, fTemplate, fSettings);

	fCancelButton = ViewFactory::Create<BButton>(frame, "cancel", _T("Cancel"), new BMessage(kAddAccountCancel));
	fOKButton = ViewFactory::Create<BButton>(frame, "ok", _T("OK"), new BMessage(kAddAccountOk));

#ifdef __HAIKU__
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	fAccountNameDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	//fProtocolControl->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

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
			.Add(fCancelButton)
			.Add(fOKButton)
		.End()
		.SetInsets(inset, inset, inset, inset)
	);

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	GetLayout()->AddView(view);
#else
	view->AddChild(fAccountName);
	view->AddChild(fAccountNameDivider);
	
	view->AddChild(fProtocolControl);
	
	view->AddChild(fCancelButton);
	view->AddChild(fOKButton);
	AddChild(view);
	
	LayoutGUI();
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

#ifndef __HAIKU__

void PAccountDialog::LayoutGUI(void) {
	font_height fh;
	BFont headingFont(be_bold_font);
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BRect frame = Bounds();

	// Account Name
	fAccountName->ResizeToPreferred();
	fAccountName->MoveTo(frame.left + inset, frame.top + inset);
	BRect frameAccountName = fAccountName->Frame();
	
	// Account Name Divider
	BRect frameAccountNameDivider = fAccountNameDivider->Frame();
	fAccountNameDivider->MoveTo(frameAccountNameDivider.left + inset, frameAccountName.bottom + inset);
	fAccountNameDivider->ResizeTo(frameAccountNameDivider.Width() - (inset * 2), frameAccountNameDivider.Height());
	frameAccountNameDivider = fAccountNameDivider->Frame();
	
	// OK Button
	fOKButton->ResizeToPreferred();
	BRect frameOKButton = fOKButton->Frame();
	fOKButton->MoveTo(frame.right - frameOKButton.Width(), frame.bottom - frameOKButton.Height());
	frameOKButton = fOKButton->Frame();
	
	// Cancel Button
	fCancelButton->ResizeToPreferred();
	BRect frameCancelButton = fCancelButton->Frame();
	fCancelButton->MoveTo(frameOKButton.left - inset - frameCancelButton.Width(), frame.bottom - frameCancelButton.Height());
	frameCancelButton = fCancelButton->Frame();

	// Protocol View
	fProtocolControl->ResizeToPreferred();
	BRect frameProtocolControl = fProtocolControl->Frame();
	fProtocolControl->MoveTo(frame.left, frameAccountNameDivider.bottom + inset);
//	fProtocolControl->ResizeTo(frame.right, frameOKButton.bottom - inset - (frameAccountNameDivider.bottom + inset));
	frameProtocolControl = fProtocolControl->Frame();
}

#endif

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
