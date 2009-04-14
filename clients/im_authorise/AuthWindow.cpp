#include "AuthWindow.h"
#include "AuthApp.h"
#include "MultiLineStringView.h"

#include <Box.h>
#include <Button.h>
#include <Screen.h>
#include <ScrollView.h>
#include <TextView.h>
#include <View.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

//#pragma mark Constants

const float kPadding = 10.0;

//#pragma mark Constructors

AuthWindow::AuthWindow(entry_ref contact, const char *reason) 
	: BWindow(BRect(0, 0, 400, 300), "im_authorise", B_TITLED_WINDOW,
		 B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE),
	fContact(&contact) {

	char name[512];
	char nick[512];
	
	if (fContact.GetName(name, sizeof(name)) != B_OK) {
		strcpy(name, _T("Unknown Name"));
	};
	if (fContact.GetNickname(nick, sizeof(nick)) != B_OK) {
		strcpy(nick, _T("Unknown Nick"));
	};

	BString title = name;
	title << " (" << nick << ")";
	SetTitle(title.String());

	fView = new BView(Bounds(), "ParentView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fView->SetViewUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fView->SetLowUIColor(B_UI_PANEL_BACKGROUND_COLOR);
	fView->SetHighUIColor(B_UI_PANEL_TEXT_COLOR);
#else
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(0, 0, 0, 0);
#endif	

	AddChild(fView);
	fView->Show();

	BMessage *auth = new BMessage(kAuthorise);
	auth->AddRef("contact", &contact);

	fAuthorise = new BButton(Bounds(), "btnAuthorise", _T("Authorise"), auth,
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fAuthorise->MakeDefault(true);
	fAuthorise->ResizeToPreferred();
	fView->AddChild(fAuthorise);
	fAuthorise->SetTarget(be_app);
	
	BRect rect = fAuthorise->Bounds();
	
	fAuthorise->MoveTo(Bounds().right - kPadding - rect.Width(),
		Bounds().bottom - kPadding - rect.Height());

	fDeny = new BButton(Bounds(), "btnDeny", _T("Deny"),
		new BMessage(B_QUIT_REQUESTED), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	fDeny->ResizeToPreferred();
	fView->AddChild(fDeny);
	
	rect = fAuthorise->Frame();
	float denyOffset = (rect.Height() - fDeny->Bounds().Height()) / 2;
		
	fDeny->MoveTo(rect.left - kPadding - fDeny->Bounds().Width(),
		Bounds().bottom - kPadding - rect.Height() + denyOffset);

	rect = Bounds();
	rect.bottom = fAuthorise->Frame().top;
	rect.InsetBy(kPadding, kPadding);

	fBox = new BBox(rect, "boxMain", B_FOLLOW_ALL_SIDES);
	fView->AddChild(fBox);
	fBox->MoveTo(kPadding, kPadding);
	fBox->SetLabel(_T("Request Details"));


	BString info = name;
	info << " (" << nick << ") has requested authorisation to add you to their "
		" contact list.\n\nAn additional message may follow.";
	fInfoText = new MultiLineStringView("txtInformation", _T(info.String()),
		fBox->Bounds().Width() - (kPadding * 2));
	fBox->AddChild(fInfoText);
	fInfoText->ResizeToPreferred();
	fInfoText->MoveTo(kPadding, kPadding * 1.5);

	BRect addRect = fBox->Bounds();
	addRect.top = fInfoText->Frame().bottom;
	addRect.right -= B_V_SCROLL_BAR_WIDTH;
	addRect.InsetBy(kPadding, kPadding);

	BRect textRect = addRect;
	textRect.OffsetTo(B_ORIGIN);
	textRect.InsetBy(be_plain_font->StringWidth("W"), be_plain_font->StringWidth("W"));
	
	fAdditional = new BTextView(addRect, "txtAdditional", textRect, B_FOLLOW_ALL,
		B_WILL_DRAW);
	fAdditional->SetWordWrap(true);
	fAdditional->SetStylable(false);
	fAdditional->MakeSelectable(true);
	fAdditional->SetText(reason);
	fAdditional->MakeEditable(false);
	
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fAdditional->SetViewUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fAdditional->SetLowUIColor(B_UI_DOCUMENT_BACKGROUND_COLOR);
	fAdditional->SetHighUIColor(B_UI_DOCUMENT_TEXT_COLOR);
#else
	fAdditional->SetViewColor(245, 245, 245, 0);
	fAdditional->SetLowColor(245, 245, 245, 0);
	fAdditional->SetHighColor(0, 0, 0, 0);
#endif

	fAdditionalScroll = new BScrollView("scrlAdditional", fAdditional,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, 0, false, true);
	fBox->AddChild(fAdditionalScroll);	

	BRect screen = BScreen().Frame();
	MoveTo((screen.Width() / 2) - (Bounds().Width() / 2),
		(screen.Height() / 2) - (Bounds().Height() / 2));

	Show();
};

AuthWindow::~AuthWindow(void) {
};

//#pragma mark BWindow Hooks

void AuthWindow::MessageReceived(BMessage *msg) {
	BWindow::MessageReceived(msg);
};

bool AuthWindow::QuitRequested(void) {
	return BWindow::QuitRequested();
};

//#pragma mark Private
const char *AuthWindow::TranslateString(const char *str, ...) {
	const char *buffer[4096];
	memset(buffer, '\0', sizeof(buffer));
#ifdef ZETA
//	va_list args;
//	va_start(args, str);
#else
	BString temp = str;
	va_list args;
	va_start(args, str);
	
//	while (
#endif
};
