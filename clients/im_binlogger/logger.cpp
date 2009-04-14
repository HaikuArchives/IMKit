#include "logger.h"

#include <Entry.h>

#include <libim/Constants.h>
#include <libim/Contact.h>
#include <libim/Helpers.h>

int main(void) {
	LoggerApp app;
	
	app.Run();
	
	return 0;
}

LoggerApp::LoggerApp()
:	BApplication("application/x-vnd.BeClan.im_binlogger") {
	fMan = new IM::Manager(this);
	
	srand(system_time());

	
	fMan->StartListening();

	// Save settings template
	BMessage autostart;
	autostart.AddString("name", "auto_start");
	autostart.AddString("description", "Auto-start");
	autostart.AddInt32("type", B_BOOL_TYPE);
	autostart.AddBool("default", true);
	
	BMessage appsig;
	appsig.AddString("name", "app_sig");
	appsig.AddString("description", "Application signature");
	appsig.AddInt32("type", B_STRING_TYPE);
	appsig.AddString("default", "application/x-vnd.BeClan.im_binlogger");
	
	BMessage tmplate(IM::SETTINGS_TEMPLATE);
	tmplate.AddMessage("setting", &autostart);
	tmplate.AddMessage("setting", &appsig);
	
	im_save_client_template("im_binlogger", &tmplate);
	
	// Make sure default settings are there
	BMessage settings;
	bool temp;
	im_load_client_settings("im_binlogger", &settings);
	if ( !settings.FindString("app_sig") )
		settings.AddString("app_sig", "application/x-vnd.BeClan.im_binlogger");
	if ( settings.FindBool("auto_start", &temp) != B_OK )
		settings.AddBool("auto_start", true );
	im_save_client_settings("im_binlogger", &settings);
	// done with template and settings.
	
	fLogParent = "/boot/home/Logs/IM/binlog/";
	create_directory(fLogParent.String(), 0777);	
}

LoggerApp::~LoggerApp() {
	fMan->Lock();
	fMan->Quit();
}

void
LoggerApp::MessageReceived(BMessage * msg) {
	switch ( msg->what ) {
		case 'newc':
		case IM::MESSAGE: {
			int32 im_what = 0;
			if (msg->FindInt32("im_what", &im_what) == B_ERROR) return;

			if ((im_what == IM::MESSAGE_SENT) || (im_what == IM::MESSAGE_RECEIVED)
				|| (im_what == IM::STATUS_CHANGED)) {
			} else {
				return;
			};
	
			entry_ref ref;
		
			if ( msg->FindRef("contact",&ref) != B_OK )	return;
			IM::Contact c(&ref);

			BString logFile = fLogParent;
			char datestr[16];
			time_t now = time(NULL);
			strftime(datestr, sizeof(datestr),"%Y%m%d.binlog", localtime(&now));
			
			BString userLogDir = "";
			int32 length = 0;
			char *attr = ReadAttribute(BNode(&ref), "IM:binarylog", &length);
			if (attr) userLogDir.Prepend(attr, length);
			free(attr);

			if (userLogDir.Length() == 0) {
				char connection[512];							
				bool canExit = false;
				BString val = "";

				c.ConnectionAt(0, connection);

				while (canExit == false) {
					val = "";
					val << connection << "_";
					val << (uint32)localtime(&now) << "_";
					val << (rand() % 32413421);

					BString temp = fLogParent;
					temp << val.String();
					BDirectory test(temp.String());
					if (test.InitCheck() != B_OK) canExit = true;
				};
				WriteAttribute(BNode(&ref), "IM:binarylog", val.String(),
						val.Length() - 1, B_STRING_TYPE);
				userLogDir = val.String();
			};
			
			logFile << userLogDir << "/" << datestr;
			
			BFile file(logFile.String(), B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);
			if (file.InitCheck() != B_OK) {
				logFile = fLogParent;
				logFile << userLogDir;
				if (create_directory(logFile.String(), 0777) != B_OK) return;
				
				logFile << "/" << datestr;
				file = BFile(logFile.String(), B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);
				if (file.InitCheck() != B_OK) return;
			};

			msg->AddInt32("time_t", (int32)time(NULL));

			ssize_t bytes;
			msg->Flatten(&file, &bytes);

			file.Unset();
			
		}	break;
		default:
			BApplication::MessageReceived(msg);
	}
};
