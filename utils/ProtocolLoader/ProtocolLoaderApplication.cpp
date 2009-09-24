#include "ProtocolLoaderApplication.h"

#include <server/Constants.h>

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Protocol.h>
#include <libim/Helpers.h>

#include "ProtocolHandler.h"

using namespace IM;

//#pragma mark Constructors

ProtocolLoaderApplication::ProtocolLoaderApplication(const char *instanceID, Protocol *protocol, const char* path, BMessage settings, const char *accountName)	
	: BApplication(IM_PROTOCOL_LOADER_SIG),
	fInstanceID(instanceID),
	fProtocol(protocol),
	fProtoName(path),
	fSettings(settings),
	fAccountName(accountName),
	fManager(NULL) {

	if (protocol != NULL) {
		BString name = fProtocol->GetFriendlySignature();
		name << " (" << fAccountName << ")";
		rename_thread(find_thread(NULL), name.String());
	};
};

ProtocolLoaderApplication::~ProtocolLoaderApplication(void) {
	delete fProtocol;

//	if (fManager) fManager->Quit();
};

//#pragma mark BApplication Hooks

void ProtocolLoaderApplication::ReadyToRun(void) {
	BMessage settings;

	im_load_protocol_template(fProtoName.String(), &settings);

	BMessage reg(IM::Private::PROTOCOL_STARTED);
	reg.AddString("instance_id", fInstanceID);
	reg.AddString("signature", fProtocol->GetSignature());
	
	reg.AddString("friendly_signature", fProtocol->GetFriendlySignature());
	reg.AddInt32("capabilities", fProtocol->Capabilities());
	reg.AddInt32("encoding", fProtocol->GetEncoding());
	reg.AddMessage("template", &settings);
	reg.AddMessenger("messenger", BMessenger(this));

	fManager = new IM::Manager(this);
	fManager->SendMessage(&reg);
	
	ProtocolHandler *handler = new ProtocolHandler(fInstanceID.String());
	AddHandler(handler);
	
	fProtocol->Init(BMessenger(handler, this));
	
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
	
