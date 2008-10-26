#include "PApplication.h"

PApplication::PApplication(void)
	: BApplication("application/x-vnd.beclan-IMKitPrefs") {
};

PApplication::~PApplication(void) {
//	delete fWindow;
};

void PApplication::ReadyToRun(void) {
	fWindow = new PWindow();
};

bool PApplication::QuitRequested(void) {
	return true;
};

void PApplication::MessageReceived(BMessage *msg) {
	//msg->PrintToStream();
	BApplication::MessageReceived(msg);

//	switch (msg->what) {
//		case PRIMARY:
//		case SECONDARY:
//		case TERTIARY:
//		case KEY_SET: {
//
//			BMessenger msgr(fWindow);
//			msgr.SendMessage(msg);
//
//		} break;
//		default: {
//			BApplication::MessageReceived(msg);
//		};
//	};
};

