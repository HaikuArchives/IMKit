#include "ProtocolLoaderApplication.h"

#include "Private/Constants.h"

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Protocol.h>

//#pragma mark Constructors

ProtocolLoaderApplication::ProtocolLoaderApplication(const char *instanceID, Protocol *protocol,
	BMessage settings, status_t *error)
	
	: BApplication(IM_PROTOCOL_LOADER_SIG),
	fInstanceID(instanceID),
	fProtocol(protocol),
	fSettings(settings),
	fManager(NULL) {
	
	if (protocol == NULL) {
		*error = B_ERROR;
	} else {
		rename_thread(find_thread(NULL), fProtocol->GetFriendlySignature());
	};
};

ProtocolLoaderApplication::~ProtocolLoaderApplication(void) {
	delete fProtocol;

//	if (fManager) fManager->Quit();
};

//#pragma mark BApplication Hooks

void ProtocolLoaderApplication::ReadyToRun(void) {
	BMessage reg(IM::Private::PROTOCOL_STARTED);
	reg.AddString("instance_id", fInstanceID);
	reg.AddString("signature", fProtocol->GetSignature());
	reg.AddString("friendly_signature", fProtocol->GetFriendlySignature());
	reg.AddInt32("capabilities", fProtocol->Capabilities());
	reg.AddInt32("encoding", fProtocol->GetEncoding());
	reg.AddMessenger("messenger", BMessenger(this));

	fManager = new IM::Manager(this);
	fManager->SendMessage(&reg);
	
	fProtocol->Init(BMessenger(IM_SERVER_SIG));
	
	BApplication::ReadyToRun();
};

bool ProtocolLoaderApplication::QuitRequested(void) {
	BMessage unreg(IM::Private::PROTOCOL_STOPPED);
	unreg.AddString("instance_id", fInstanceID);
	unreg.AddString("signature", fProtocol->GetSignature());
	unreg.AddString("friendly_signature", fProtocol->GetFriendlySignature());
	
	fManager->SendMessage(&unreg);
	fProtocol->Shutdown();

	return BApplication::QuitRequested();
};

void ProtocolLoaderApplication::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case IM::Private::PROTOCOL_PROCESS: {
			BMessage inner;
			if (msg->FindMessage("message", &inner) != B_OK) return;

			fProtocol->Process(&inner);
		} break;

		case IM::Private::PROTOCOL_UPDATESETTINGS: {
			BMessage inner;
			if (msg->FindMessage("message", &inner) != B_OK) return;

			fProtocol->UpdateSettings(inner);
		} break;

		default: {
			BApplication::MessageReceived(msg);
		} break;
	};
};
	
