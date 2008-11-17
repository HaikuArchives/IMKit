#include <Application.h>
#include <Directory.h>
#include <kernel/fs_attr.h>
#include <Node.h>
#include <Path.h>
#include <image.h>

#include <libim/Helpers.h>
#include <libim/Manager.h>
#include <libim/Protocol.h>

#include <stdio.h>
#include <signal.h>

#include "ProtocolLoaderApplication.h"
#include "Private/Constants.h"

// #pragma mark Constants

const char *kApplicationName = "ProtocolLoader";

using namespace IM;

BString gInstanceID;

void appKilled(int /*sig*/) {
	LOG(kApplicationName, liHigh, "Application Killed - notifying server %s", gInstanceID.String());
	
	static bool quitRequested = false;
	if (quitRequested == true) {
		BMessage crashed(IM::Private::PROTOCOL_KILLED);
		crashed.AddString("instance_id", gInstanceID);
	
		IM::Manager manager;	
		manager.SendMessage(&crashed);
	
		BMessenger(be_app).SendMessage(B_QUIT_REQUESTED);
		quitRequested = true;
		
		snooze(1000 * 1000);
	} else {
		exit(B_ERROR);
	};
}

void SendNoStartMessage(const char *instanceID, const char *reason) {
	IM::Manager manager;
	BMessage msg(IM::Private::PROTOCOL_COULD_NOT_START);
	msg.AddString("instance_id", instanceID);
	if (reason != NULL) msg.AddString("reason", reason);
	
	manager.SendMessage(&msg);
};

int main(int argc, char *argv[]) {
	// Check that we have a protocol path
	if (argc < 4) {
		LOG(kApplicationName, liHigh, "Incorrect usage of ProtocolLoader. Correct usage is: %s {instance identifier} {path to protocol to load} {path to settings file} [{true to allow debug_server to launch}]", argv[0]);
		return B_ERROR;
	};
	
	gInstanceID = argv[1];
	BPath path(argv[2]);
	BNode settingsNode(argv[3]);
	bool allowDebugServer = false;
	status_t result = settingsNode.InitCheck();
	
	if ((argc > 4) && (strcmp(argv[4], "true") == 0)) {
		allowDebugServer = true;
	};
	
	if (result != B_OK) {
		LOG(kApplicationName, liHigh, "Settings file is not valid: %s (%i)", strerror(result), result);
		SendNoStartMessage(gInstanceID.String(), "Settings file is not valid");

		return B_ERROR;
	};

	image_id curr_image = load_add_on( path.Path() );
	if (curr_image < 0) {
		LOG(kApplicationName, liHigh, "Could not load add on: %s (%i)", strerror(curr_image), curr_image);
		SendNoStartMessage(gInstanceID.String(), "Could not load add on");
		
		return B_ERROR;
	}

	// Obtain the load_protocol function
	Protocol *(*load_protocol)(void);
	result = get_image_symbol(curr_image, "load_protocol", B_SYMBOL_TYPE_TEXT, (void **)&load_protocol);
	
	if (result != B_OK) {
		LOG(kApplicationName, liHigh, "Could not load symbol (load_protocol): %s (%i)", strerror(result), result);
		SendNoStartMessage(gInstanceID.String(), "Add-on is invalid");

		unload_add_on(curr_image);		
		return B_ERROR;
	}

	Protocol *protocol = load_protocol();
	
	if (protocol == NULL) {
		LOG(kApplicationName, liHigh, "Could not instantiate Protocol");
		SendNoStartMessage(gInstanceID.String(), "Could not instantiate Protocol");

		unload_add_on(curr_image);
		return B_ERROR;
	}
	
	LOG(kApplicationName, liMedium, "Protocol loaded: %s", protocol->GetSignature());

	// try to read settings from protocol attribute
	BMessage settings;
	attr_info info;

	status_t rc = settingsNode.GetAttrInfo("im_settings", &info);

	if ((rc == B_OK) && (info.type == B_RAW_TYPE) && (info.size > 0)) {
		// found an attribute with data
		char *buffer = (char *)calloc(sizeof(char), info.size);
		settingsNode.ReadAttr("im_settings", info.type, 0, buffer, info.size);

		LOG("im_server", liLow, "Read settings data");
		
		if (settings.Unflatten(buffer) == B_OK) {
			protocol->UpdateSettings(settings);
		} else {
			LOG(kApplicationName, liHigh, "Could not unflatten settings message");
			SendNoStartMessage(gInstanceID.String(), "Could not unflatten settings message");

			free(buffer);
			
			delete protocol;
			unload_add_on(curr_image);
		
			return B_ERROR;	
		};
		
		free(buffer);
	} else {
		LOG(kApplicationName, liMedium, "No settings found");
	}
		
	if (allowDebugServer == false) {
		struct sigaction killedAction;
		killedAction.sa_handler = appKilled;
		killedAction.sa_mask = 0;
		killedAction.sa_flags = 0;
		killedAction.sa_userdata = 0;
			
		sigaction(SIGSEGV, &killedAction, NULL);
	};
	
	ProtocolLoaderApplication app(gInstanceID.String(), protocol, settings);
	app.Run();
	
	return B_OK;
};
