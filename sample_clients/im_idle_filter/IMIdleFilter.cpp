#include "IMIdleFilter.h"

#include <MessageRunner.h>
#include <be/support/syslog.h>
#include <kernel/OS.h>

#include <libim/Constants.h>
#include <libim/Helpers.h>

#include <Roster.h>

//#pragma mark Constants

const int32 kMessageIdle = 'imf0';
const int32 kMessageReturn = 'imf1';

const char *kSettingName = "im_idle_filter";
const char *kDefaultAppSig = "application/x-vnd.beclan.im_filter";
const bool kDefaultAutoStart = false;
const int32 kDefaultDelay = 10;
const bigtime_t kMinuteMulti = 1000000 * 60;
const char *kDefaultAway = "My hands are busy as I'm looking at pr0n, so I'm away. Back "
	"in 30 seconds";
const char *kReturnNeverMsg = "Never";
const char *kReturnIdleMsg = "When idle";
const char *kReturnAlwaysMsg = "Always";
const char *kDefaultReturnMsg = kReturnIdleMsg;

const int32 kReturnNever = 0;
const int32 kReturnIdle = 1;
const int32 kReturnAlways = 2;
const int32 kDefaultReturn = kReturnIdle;

const int8 kStatusOffline = 0;
const int8 kStatusAway = 1;
const int8 kStatusOnline = 2;


//#pragma mark C Functions

extern "C" {
	_EXPORT BInputServerFilter *instantiate_input_filter(void) {
		return new IMIdleFilter();
	};
};

//#pragma mark Constructors

IMIdleFilter::IMIdleFilter(void)
	: BLooper("IMIdleFilter", B_LOW_PRIORITY), BInputServerFilter(), 
	fMan(NULL),
	fRunner(NULL),
	fIdle(false),
	fStatus(kStatusOffline) {

#ifdef DEBUG
	openlog_team("IMIdleFilter: ", LOG_PID | LOG_SERIAL, LOG_USER);
	setlogmask_team(LOG_UPTO(LOG_EMERG));
	
	syslog(LOG_INFO, "IMIdleFilter created\n");
#endif
};

IMIdleFilter::~IMIdleFilter(void) {
#ifdef DEBUG
	syslog(LOG_INFO, "~IMIdleFilter(%p) called\n", this);
#endif

	if (fMan) {
		fMan->Lock();
		fMan->Quit();
	};

	delete fRunner;

#ifdef DEBUG	
	syslog(LOG_INFO, "~IMIdleFilter(%p) done\n", this);
	closelog_team();
#endif
};

//#pragma mark BInputServerFilter Hooks

filter_result IMIdleFilter::Filter(BMessage *message, BList *outlist) {
	(void)message;
	(void)outlist;

	if (fRunner) delete fRunner;
	
	if (fDelay > 0) {
		fRunner = new BMessageRunner(BMessenger(this),
			new BMessage(kMessageIdle), fDelay * kMinuteMulti, 1);
	};
	
	if (fReturnType != kReturnNever) {
		bool sendOnline = false;

		// If the status is away...
		if (fStatus == kStatusAway) {
			// If we're set to return always and the status is away, send online
			if (fReturnType == kReturnAlways) sendOnline = true;
			if ((fReturnType == kReturnIdle) && (fIdle == true)) sendOnline = true;
		};
		
		if (sendOnline) BMessenger(this).SendMessage(new BMessage(kMessageReturn));
	};
	
	fIdle = false;
	
	return B_DISPATCH_MESSAGE;
};

status_t IMIdleFilter::InitCheck(void) {
	Run();

	fMan = new IM::Manager(BMessenger(this));
	
	if (fMan->InitCheck() != B_OK) {
#ifdef DEBUG
		syslog(LOG_INFO, "IMIdleFilter::InitCheck()\tIM::Manager could not init - "
			" \"%s\" (%i)\n", strerror(fMan->InitCheck()), fMan->InitCheck());
#endif
		be_roster->StartWatching(BMessenger(this));
	};
	
	SaveSettingsTemplate();

	return B_OK;
};

//#pragma mark BLooper Hooks

void IMIdleFilter::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case B_SOME_APP_LAUNCHED: {
			const char *signature = NULL;
			if (msg->FindString("be:signature", &signature) != B_OK) return;

#ifdef DEBUG
			syslog(LOG_INFO, "IMIdleFilter\t%s started\n", signature);
#endif
			
			if (strcmp(signature, IM_SERVER_SIG) == 0) {
#ifdef DEBUG
				syslog(LOG_INFO, "IMIdleFilter\tim_server started! Constructing "
					"an IM::Manager...");
#endif
			
				snooze(1000*1000);

				fMan = new IM::Manager(BMessenger(this));
				if (fMan->InitCheck() == B_OK) {
					fMan->StartListening();
				
					// We've got a manager, we can stop watching
					be_roster->StopWatching(BMessenger(this));
#ifdef DEBUG
					syslog(LOG_INFO, "IMIdleFilter\tIM::Manager constructed, "
						"app watching stopped\n");
				} else {
					syslog(LOG_INFO, "IMIdleFilter\tIM::Manager failed InitCheck");
#endif
				};			
			};
				
		} break;
		case IM::SETTINGS_UPDATED: {
			LoadSettings();

			if (fRunner) fRunner->SetInterval(fDelay * kMinuteMulti);

#ifdef DEBUG
			syslog(LOG_INFO, "Settings updated, delay is now %i minutes\n", fDelay);
#endif
		} break;
	
		case IM::MESSAGE: {
			int32 im_what = B_ERROR;
			if (msg->FindInt32("im_what", &im_what) != B_OK) return;
			
			switch (im_what) {
				case IM::STATUS_SET: {
					const char *status = msg->FindString("total_status");
					if (status == NULL) return;
					
					if (strcmp(status, ONLINE_TEXT) == 0) fStatus = kStatusOnline;
					if (strcmp(status, AWAY_TEXT) == 0) fStatus = kStatusAway;
					if (strcmp(status, OFFLINE_TEXT) == 0) fStatus = kStatusOffline;
				} break;
			};
			
		} break;
	
		case kMessageIdle: {
#ifdef DEBUG
			syslog(LOG_INFO, "Setting idle!\n", &msg->what);
#endif

			BMessage status(IM::MESSAGE);
			status.AddInt32("im_what", IM::SET_STATUS);
			status.AddString("away_msg", fAwayMsg);
			status.AddString("status", AWAY_TEXT);
			
			fMan->SendMessage(&status);
			
			fIdle = true;
		} break;
		
		case kMessageReturn: {
#ifdef DEBUG
			syslog(LOG_INFO, "Returning from away\n");
#endif

			BMessage status(IM::MESSAGE);
			status.AddInt32("im_what", IM::SET_STATUS);
			status.AddString("status", ONLINE_TEXT);
			
			fMan->SendMessage(&status);
		} break;
		
		default: {
			BLooper::MessageReceived(msg);
		};
	};
};

//#pragma mark Private

status_t IMIdleFilter::SaveSettingsTemplate(void) {
	BMessage autostart;
	autostart.AddString("name", "auto_start");
	autostart.AddString("description", "Auto-start");
	autostart.AddInt32("type", B_BOOL_TYPE);
	autostart.AddBool("default", kDefaultAutoStart);
	autostart.AddString("help", "Should im_server start this automatically?");

	BMessage appsig;
	appsig.AddString("name", "app_sig");
	appsig.AddString("description", "Application signature");
	appsig.AddInt32("type", B_STRING_TYPE);
	appsig.AddString("default", kDefaultAppSig);

	BMessage delay;
	delay.AddString("name", "idle");
	delay.AddString("description", "Minutes before away");
	delay.AddString("help", "After this many minutes you'll be set as away");
	delay.AddInt32("type", B_INT32_TYPE);
	delay.AddInt32("default", kDefaultDelay);
	
	BMessage message;
	message.AddString("name", "away_message");
	message.AddString("description", "Away message");
	message.AddString("help", "You can set a custom away message (for supported "
		"protocols) here");
	message.AddInt32("type", B_STRING_TYPE);
	message.AddString("default", kDefaultAway);
	message.AddBool("multi_line", true);
	
	// Ack! This should be an integer, but then we can't have pretty options :(
	BMessage retIdle;
	retIdle.AddString("name", "return");
	retIdle.AddString("description", "When no longer idle set online");
	retIdle.AddString("help", "When you are no longer idle you can be set to"
		" online either\n"
		" - Never\n"
		" - Only when you were idle and set to away\n"
		" - Always (Eg. Even if you manually set yourself to away)");
	retIdle.AddInt32("type", B_STRING_TYPE);
	retIdle.AddString("default", kDefaultReturnMsg);
	retIdle.AddString("valid_value", kReturnNeverMsg);
	retIdle.AddString("valid_value", kReturnIdleMsg);
	retIdle.AddString("valid_value", kReturnAlwaysMsg);

	
	BMessage settings(IM::SETTINGS_TEMPLATE);
	settings.AddMessage("setting", &autostart);
	settings.AddMessage("setting", &appsig);
	settings.AddMessage("setting", &delay);
	settings.AddMessage("setting", &message);
	settings.AddMessage("setting", &retIdle);

	im_save_client_template(kSettingName, &settings);
	
	return B_OK;
};

status_t IMIdleFilter::LoadSettings(void) {
	BMessage settings;
	BString returnType;
	
	im_load_client_settings(kSettingName, &settings);

	if (settings.FindInt32("idle", &fDelay) != B_OK) fDelay = kDefaultDelay;
	if (settings.FindString("away_message", &fAwayMsg) != B_OK) fAwayMsg = kDefaultAway;
	if (settings.FindString("return", &returnType) != B_OK) returnType = kDefaultReturnMsg;

	fReturnType = kDefaultReturn;
	if (returnType == kReturnNeverMsg) fReturnType = kReturnNever;
	if (returnType == kReturnIdleMsg) fReturnType = kReturnIdle;
	if (returnType == kReturnAlwaysMsg) fReturnType = kReturnAlways;
	
	return B_OK;
};


