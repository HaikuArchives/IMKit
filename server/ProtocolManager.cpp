#include "ProtocolManager.h"
#include "ProtocolInfo.h"
#include "ProtocolSpecification.h"

#include "Private/Constants.h"

#include <app/Roster.h>
#include <storage/Entry.h>
#include <support/Autolock.h>
#include <support/Locker.h>

#include <libim/Constants.h>
#include <libim/Helpers.h>

#include <map>

using namespace IM;

class IM::ProtocolStore {
	public:
		ProtocolStore(void) {
		};
		
		~ProtocolStore(void) {
			Clear();
		};
		
	
		void Add(BString key, ProtocolInfo *info) {
			fStore[key] = info;
		};
		
		bool Contains(BString key) {
			return (fStore.find(key) != fStore.end());
		};
		
		ProtocolInfo *Find(BString key) {
			ProtocolInfo *info = NULL;
			map<BString, ProtocolInfo *>::iterator it = fStore.find(key);

			if (it != fStore.end()) {
				info = it->second;
			};
			
			return info;
		};
				
		void Clear(void) {
			map<BString, ProtocolInfo *>::iterator it;
			
			for (it = Start(); it != End(); it++) {
				delete it->second;
			};
			
			fStore.clear();
		};
		
		map<BString, ProtocolInfo *>::iterator Start() {
			return fStore.begin();
		};
		map<BString, ProtocolInfo *>::iterator End() {
			return fStore.end();
		};

	private:
		map<BString, ProtocolInfo *> fStore;
};

//#pragma mark Constructor

ProtocolManager::ProtocolManager(void)
	: fProtocol(new ProtocolStore()),
	fLock(new BLocker("ProtocolManager")) {
	
	entry_ref ref;
	if (be_roster->FindApp(IM_PROTOCOL_LOADER_SIG, &ref) != B_OK) {
		LOG("im_server", liHigh, "Could not find ProtocolLoader");
	};
	
	fLoaderPath = BPath(&ref);
};

ProtocolManager::~ProtocolManager(void) {
	delete fProtocol;
	delete fLock;
};

//#pragma mark Public

status_t ProtocolManager::LoadFromDirectory(BDirectory protocols, BDirectory settings) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		BPath path((const BDirectory *)&protocols, NULL, false);
		LOG("im_server", liMedium, "ProtocolManager::LoadFromDirectory called - %s", path.Path());
	
		BEntry entry;
		protocols.Rewind();
				
		while (protocols.GetNextEntry((BEntry*)&entry, true) == B_NO_ERROR) {
			// continue until no more files
			if (entry.InitCheck() != B_NO_ERROR) continue;
	
			BPath addonPath;
			entry.GetPath(&addonPath);
			
			BPath settingsPath(&settings, addonPath.Leaf());
			
			ProtocolInfo *info = new ProtocolInfo(addonPath, settingsPath);
			fProtocol->Add(info->InstanceID(), info);

			LOG("im_server", liDebug, "Loading protocol from %s (%s)", fLoaderPath.Path(),
				settingsPath.Path());
			info->Start(fLoaderPath.Path());
		};
	};
	
	return result;
};

status_t ProtocolManager::RestartProtocols(ProtocolSpecification *match, bool canDelete = true) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
		
	if (lock.IsLocked() == true) {
		map<BString, ProtocolInfo *>::iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++){
			ProtocolInfo *info = it->second;
		
			if (match->IsSatisfiedBy(info) == true) {
				info->Start(fLoaderPath.Path());
			};
		};
		
		result = B_OK;
	};
	
	if (canDelete == true) delete match;
	
	return result;
};

status_t ProtocolManager::Unload(void) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {	
		map<BString, ProtocolInfo *>::iterator it;
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *info = it->second;
			
			if (info->HasValidMessenger() == true) {
				info->Messenger()->SendMessage(B_QUIT_REQUESTED);
			} else {
				// A half loaded addon - force it to exit
				kill_thread(info->ThreadID());
			};
		};

		// Give all the protocols a chance to shutdown
		snooze(20000);
	
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *info = it->second;
			status_t exitValue = B_ERROR;
			
			status_t result = wait_for_thread(info->ThreadID(), &exitValue);
			
			LOG("im_server", liMedium, "Protocol %s/%s: Exiting - %s: %s (%i)", info->Path().Leaf(),
				info->SettingsPath().Leaf(), strerror(result), strerror(exitValue), exitValue);
		}

		fProtocol->Clear();
		result = B_OK;
	};
	
	return result;
};

status_t ProtocolManager::MessageProtocols(ProtocolSpecification *match, BMessage *message, bool canDelete = true, bool appendSignature = true) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		map<BString, ProtocolInfo *>::iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++){
			ProtocolInfo *info = it->second;
		
			if (match->IsSatisfiedBy(info) == true) {
				BMessage copy(*message);
				if (appendSignature == true) copy.AddString("protocol", info->Signature());
				info->Process(&copy);
				
				result = B_OK;
			};
		};
	};
	
	if (canDelete == true) delete match;
	
	return result;
};


ProtocolInfo *ProtocolManager::FindProtocol(ProtocolSpecification *match, bool canDelete = true) {
	ProtocolInfo *info = NULL;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		map<BString, ProtocolInfo *>::iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *cur = it->second;
			
			if (match->IsSatisfiedBy(cur) == true) {
				info = cur;
				break;
			};
		};
	};
	
	if (canDelete == true) delete match;
	
	return info;
};

list<ProtocolInfo *> ProtocolManager::FindProtocols(ProtocolSpecification *match, bool canDelete = true) {
	list<ProtocolInfo *> protocols;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		map<BString, ProtocolInfo *>::iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *info = it->second;
			
			if (match->IsSatisfiedBy(info) == true) {
				protocols.push_back(info);
			};
		};
	};
	
	if (canDelete == true) delete match;
	
	return protocols;
};

//#pragma mark Private
