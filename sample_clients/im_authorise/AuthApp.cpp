#include "AuthApp.h"

// Project
#include "AuthWindow.h"

// BeOS API
#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif
#include <Application.h>
#include <Roster.h>
#include <Path.h>

// IM
#include <libim/Constants.h>
#include <libim/Helpers.h>

// STL
#include <algorithm>

//#pragma mark Constants

const int32 kAuthorise = 'AAAu';
const int32 kDeny = 'AADe';
const char *kAppSig = "application/x-vnd.BeClan.im_authorise";

//#pragma mark Constructor

AuthApp::AuthApp(void)
	: BApplication(kAppSig) {

	fManager = new IM::Manager(BMessenger(this));

#ifdef ZETA
	app_info ai;
	BPath path;
	BString path_string;
	
	GetAppInfo(&ai);
	BEntry entry(&ai.ref, true);
	entry.GetPath(&path);
	path.GetParent(&path);
	path.Append("Language/Dictionaries/im_authorise");
	
	if (path.InitCheck() != B_OK) {
		path_string.SetTo("Language/Dictionaries/im_authorise");
	} else {
		path_string.SetTo(path.Path());
	};
	
	be_locale.LoadLanguageFile(path_string.String());
#endif

	UpdateSettings();
};

AuthApp::~AuthApp(void) {
	fManager->Lock();
	fManager->Quit();
};

//#pragma mark BApplication Hooks

void AuthApp::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case IM::MESSAGE: {
			entry_ref ref;
			int32 im_what = B_ERROR;
			const char *reason = NULL;
			
			if (msg->FindRef("contact", &ref) != B_OK) return;
			if (msg->FindInt32("im_what", &im_what) != B_OK) return;
			if (im_what != IM::AUTH_REQUEST) return;
				
			new AuthWindow(ref, reason);
		} break;
		
		case kAuthorise: {
			entry_ref ref;
			if (msg->FindRef("contact", &ref) != B_OK) return;
		
			BMessage *authReply = new BMessage(IM::MESSAGE);
			authReply->AddInt32("im_what", IM::SEND_AUTH_ACK);
			authReply->AddRef("contact", &ref);
			
			fManager->SendMessage(authReply);
		} break;
		
		case kDeny: {
		} break;
		
		default: {
			BApplication::MessageReceived(msg);
		};
	};
};

//#pragma mark Private

void AuthApp::UpdateSettings(void) {
	BMessage autostart;
	autostart.AddString("name", "auto_start");
	autostart.AddString("description", "Auto-start");
	autostart.AddInt32("type", B_BOOL_TYPE);
	autostart.AddBool("default", true);
	autostart.AddString("help", "Should im_server start this automatically?");
	
	BMessage appsig;
	appsig.AddString("name", "app_sig");
	appsig.AddString("description", "Application signature");
	appsig.AddInt32("type", B_STRING_TYPE);
	appsig.AddString("default", "application/x-vnd.BeClan.im_authorise");

	BMessage tmplate(IM::SETTINGS_TEMPLATE);
	tmplate.AddMessage("setting", &autostart);
	tmplate.AddMessage("setting", &appsig);
	
	im_save_client_template("im_authorise", &tmplate);
	
	// Make sure default settings are there
	BMessage settings;
	bool temp;
	
	im_load_client_settings("im_authorise", &settings);
	if (!settings.FindString("app_sig")) settings.AddString("app_sig", kAppSig);
	if (settings.FindBool("auto_start", &temp) != B_OK) settings.AddBool("auto_start", true );

	im_save_client_settings("im_authorise", &settings);
};