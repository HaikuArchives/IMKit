#include "ProtocolInfo.h"

#include "Private/Constants.h"

#include <app/Messenger.h>
#include <kernel/image.h>

using namespace IM;

extern char **environ;

//#pragma mark Constructor

ProtocolInfo::ProtocolInfo(BPath path, BPath settings)
	: fPath(path),
	fSettingsPath(settings),
	fSignature(""),
	fFriendlySignature(""),
	fCapabilities(0),
	fEncoding(0xffff),
	fThreadID(B_ERROR),
	fMessenger(NULL),
	fAllowRestart(false) {

	
	fInstanceID = fPath.Path();
	fInstanceID << "->";
	fInstanceID << fSettingsPath.Path();
};

ProtocolInfo::~ProtocolInfo(void) {
	delete fMessenger;
};

//#pragma mark Accessor Methods

BPath ProtocolInfo::Path(void) {
	return fPath;
};

BPath ProtocolInfo::SettingsPath(void) {
	return fSettingsPath;
};

const char *ProtocolInfo::InstanceID(void) {
	return fInstanceID.String();
};

thread_id ProtocolInfo::ThreadID(void) {
	return fThreadID;
};

const char *ProtocolInfo::Signature(void) {
	return fSignature.String();
};

void ProtocolInfo::Signature(const char *signature) {
	fSignature = signature;
};

const char *ProtocolInfo::FriendlySignature(void) {
	return fFriendlySignature.String();
};

void ProtocolInfo::FriendlySignature(const char *signature) {
	fFriendlySignature = signature;
};

uint32 ProtocolInfo::Capabilities(void) {
	return fCapabilities;
};

void ProtocolInfo::Capabilities(uint32 caps) {
	fCapabilities = caps;
};

uint32 ProtocolInfo::Encoding(void) {
	return fEncoding;
};

void ProtocolInfo::Encoding(uint32 encoding) {
	encoding = fEncoding;
};

BMessage ProtocolInfo::SettingsTemplate(void) {
	return fSettingsTemplate;
};

void ProtocolInfo::SettingsTemplate(BMessage settings) {
	fSettingsTemplate = settings;
};

BMessenger *ProtocolInfo::Messenger(void) {
	return fMessenger;
};

void ProtocolInfo::Messenger(BMessenger *messenger) {
	if (fMessenger) delete fMessenger;
	fMessenger = messenger;
};

//#pragma mark Informational Methods

bool ProtocolInfo::HasCapability(uint32 capability) {
	return ((fCapabilities & capability) != 0);
};

bool ProtocolInfo::HasExited(void) {
	thread_info ignored;
	return ((fThreadID == B_ERROR) || (get_thread_info(fThreadID, &ignored) == B_BAD_THREAD_ID));
};

bool ProtocolInfo::HasValidMessenger(void) {
	return ((fMessenger != NULL) && (fMessenger->IsValid()));
};

bool ProtocolInfo::IsRestartAllowed(void) {
	return fAllowRestart;
};

//#pragma mark Control Methods

status_t ProtocolInfo::Start(const char *loader) {
	status_t result = B_ERROR;

	fAllowRestart = true;

	const char *arguments[] = {
		loader,						// Path to the ProtocolLoader
		fInstanceID.String(),		// Instance ID
		fPath.Path(),				// Path to Protocol Addon
		fSettingsPath.Path(),		// Path to settings
		NULL
	};
	
	fThreadID = load_image(4, arguments, (const char **)environ);
	
	if (fThreadID > B_ERROR) {
		resume_thread(fThreadID);
		result = B_OK;
	};
	
	return result;
};

void ProtocolInfo::Stop(void) {
	if (fMessenger) delete fMessenger;
	
	fMessenger = new BMessenger();
	fThreadID = B_ERROR;
	
	fAllowRestart = false;
};

//#pragma mark Message Methods

status_t ProtocolInfo::Process(BMessage *msg) {
	BMessage envelope(IM::Private::PROTOCOL_PROCESS);
	envelope.AddMessage("message", msg);
	
	fMessenger->SendMessage(&envelope);

	return B_OK;								
};

status_t ProtocolInfo::UpdateSettings(BMessage *msg) {
	BMessage envelope(IM::Private::PROTOCOL_UPDATESETTINGS);
	envelope.AddMessage("message", msg);
	
	fMessenger->SendMessage(&envelope);

	return B_OK;
};
