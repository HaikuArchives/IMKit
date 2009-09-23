#include <interface/Button.h>
#include <interface/ScrollView.h>
#include <interface/TextView.h>
#include <interface/View.h>
#include <support/String.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <libim/Manager.h>

#include "AwayMessageWindow.h"

//#pragma mark Constants

const int32 kMsgCancelAway = 'mcaw';
const int32 kMsgSetAway = 'msaw';
const float kPadding = 5.0f;

//#pragma mark Constructor

AwayMessageWindow::AwayMessageWindow(IM::AccountInfo *info)
	: BWindow(BRect(100, 100, 325, 220), _T("IM Kit: Set Away Message"), B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS),
	fAccountInstance("") {

	if (info != NULL) {
		fAccountInstance = info->ID();
	
		BString t = _T("Set away message");
		t << ": ";
		t.Append(info->DisplayLabel(), strlen(info->DisplayLabel()));

		SetTitle(t.String());
	};

	font_height height;
	be_plain_font->GetHeight(&height);
	fFontHeight = height.ascent + height.descent + height.leading;

	BRect rect = Bounds();

	fView = new BView(rect, "AwayView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fView->SetViewUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fView->SetLowUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fView->SetHighUIColor(B_UI_PANEL_TEXT_COLOR);
#else
	fView->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	fView->SetLowColor( ui_color(B_PANEL_BACKGROUND_COLOR) );
	fView->SetHighColor(0, 0, 0, 255);
#endif	
	AddChild(fView);

	rect.InsetBy(kPadding, kPadding);
	rect.bottom -= (fFontHeight + (kPadding * 3));
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	
	BRect textRect = rect.InsetBySelf(2.0, 2.0);
	textRect.OffsetTo(0, 0);
	
	fTextView = new BTextView(rect, "AwayMsg", textRect, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fTextView->SetViewUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fTextView->SetLowUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fTextView->SetHighUIColor(B_UI_DOCUMENT_TEXT_COLOR);
#else
	fTextView->SetViewColor(245, 245, 245, 0);
	fTextView->SetLowColor(245, 245, 245, 0);
	fTextView->SetHighColor(0, 0, 0, 0);
#endif

	fScroller = new BScrollView("AwayScroller", fTextView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, false, true);
	fView->AddChild(fScroller);
	
	rect = Bounds();
	rect.InsetBy(kPadding, kPadding);
	
	rect.left = rect.right - (be_plain_font->StringWidth(_T("Set Away")) + (kPadding * 2));
	rect.bottom -= kPadding;
	rect.top = rect.bottom - (fFontHeight + kPadding);
	
	fOkay = new BButton(rect, "OkayButton", _T("Set Away"), new BMessage(kMsgSetAway), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fView->AddChild(fOkay);
	
	rect.right = rect.left - kPadding;
	rect.left = rect.right - (be_plain_font->StringWidth(_T("Cancel")) + (kPadding * 2));
	
	fCancel = new BButton(rect, "CancelButton", _T("Cancel"), new BMessage(kMsgCancelAway), B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	fView->AddChild(fCancel);

	BMessage settings;
	im_load_client_settings("im_server", &settings);

	BString	awayMsg = "I'm not here";
		
	if (settings.FindString("default_away", &awayMsg) == B_OK) {
		fTextView->SetText(awayMsg.String());
	};
	
	fTextView->MakeFocus(true);
}

AwayMessageWindow::~AwayMessageWindow(void) {
/*	if (fScroller) 
		fScroller->RemoveSelf();
	delete fScroller;
	
	delete fTextView;
	
	if (fOkay) 
		fOkay->RemoveSelf();
	delete fOkay;
	
	if (fCancel) 
		fCancel->RemoveSelf();
	delete fCancel;
	
	if (fView) 
		fView->RemoveSelf();
	
	delete fView;
*/
};

//#pragma mark BWindow Hooks

bool AwayMessageWindow::QuitRequested(void) {
	return true;
};

void AwayMessageWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kMsgCancelAway: {
			BMessenger(this).SendMessage(B_QUIT_REQUESTED);
		} break;
		
		case kMsgSetAway: {
			BMessage status(IM::MESSAGE);
			status.AddInt32("im_what", IM::SET_STATUS);
			if (fAccountInstance.Length() > 0) {
				status.AddString("instance_id", fAccountInstance);
			};
			status.AddString("away_msg", fTextView->Text());
			status.AddString("status", AWAY_TEXT);
			
			IM::Manager man;
			
			man.OneShotMessage(&status);
			
			BMessenger(this).SendMessage(B_QUIT_REQUESTED);
		} break;
	
		default: {
			BWindow::MessageReceived(msg);
		};
	};
};
