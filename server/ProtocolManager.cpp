#include "ProtocolManager.h"
#include "ProtocolInfo.h"
#include "ProtocolSpecification.h"

#include "common/GenericStore.h"
#include "Constants.h"

#include <app/Roster.h>
#include <storage/VolumeRoster.h>
#include <storage/Volume.h>
#include <storage/Query.h>
#include <storage/Entry.h>
#include <support/Autolock.h>
#include <support/Locker.h>

#include <libim/Constants.h>
#include <libim/Helpers.h>

#include <map>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

using namespace IM;

//#pragma mark Constants

const bigtime_t kQueryDelay = 5 * 1000 * 1000;	// 5 Seconds

class IM::ProtocolStore : public IM::GenericMapStore<BString, ProtocolInfo *> {
};

//#pragma mark Constructor

ProtocolManager::ProtocolManager(void)
	: fProtocol(new ProtocolStore()),
	fLock(new BLocker("ProtocolManager")) {
	
	LOG("im_server", liHigh, "Finding ProtocolLoader with signature "IM_PROTOCOL_LOADER_SIG);

	entry_ref ref;
	if (be_roster->FindApp(IM_PROTOCOL_LOADER_SIG, &ref) != B_OK) {
		// Try with a query and take the first result
		BVolumeRoster vroster;
		BVolume volume;
		char volName[B_FILE_NAME_LENGTH];

		vroster.Rewind();

		while (vroster.GetNextVolume(&volume) == B_OK) {
			if ((volume.InitCheck() != B_OK) || (volume.KnowsQuery() == false))
				continue;

			volume.GetName(volName);
			LOG("im_server", liDebug, "ProtocolManager: Finding ProtocolLoader with a query on %s", volName);

			BQuery* query = new BQuery();
			query->SetPredicate("(BEOS:APP_SIG==\""IM_PROTOCOL_LOADER_SIG"\")");
			query->SetVolume(&volume);
			query->Fetch();

			if (query->GetNextRef(&ref) == B_OK) {
				break;
			}
			
			LOG("im_server", liHigh, "Unable to find ProtocolLoader - waiting before retrying");
			snooze(kQueryDelay);
				
			if (query->GetNextRef(&ref) == B_OK) {			
				break;
			};
		}
	}
	fLoaderPath = BPath(&ref);

	LOG("im_server", liHigh, "ProtocolLoader path: %s", fLoaderPath.Path());
};

ProtocolManager::~ProtocolManager(void) {
	delete fProtocol;
	delete fLock;
};

status_t ProtocolManager::InitCheck()
{
	if (fLoaderPath.Path())
		return B_OK;
	return B_ERROR;
}

//#pragma mark Public

ProtocolInfo *ProtocolManager::LaunchInstance(BPath protocolPath, BPath settingsPath, const char *name) {
	ProtocolInfo *info = new ProtocolInfo(protocolPath, settingsPath, name);
	fProtocol->Add(info->InstanceID(), info);
	info->Start(fLoaderPath.Path());
	
	return info;
};

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
			
			BMessage accounts;
			BString account;
			im_protocol_get_account_list(addonPath.Leaf(), &accounts);

			for (int32 i = 0; accounts.FindString("account", i, &account) == B_OK; i++) {
				entry_ref accountSettings;
				
				if (accounts.FindRef("settings_path", i, &accountSettings) != B_OK) {
					LOG("im_server", liHigh, "%s - %s does not have a settings path", settingsPath.Leaf(), account.String());
					continue;
				};
				
				BPath accountSettingsPath(&accountSettings);
				LaunchInstance(addonPath, accountSettingsPath, account.String());

				LOG("im_server", liMedium, "Loading protocol from %s (%s): %s", fLoaderPath.Path(), settingsPath.Path(), account.String());
			};
		};
	};
	
	return result;
};


status_t ProtocolManager::RestartProtocols(ProtocolSpecification *match, bool canDelete) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
		
	if (lock.IsLocked() == true) {
		GenericMapStore<BString, ProtocolInfo *>::Iterator it;
		
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

status_t ProtocolManager::UnloadInstance(ProtocolInfo *info) {
	return UnloadInstance(info, true);
};

status_t ProtocolManager::Unload(void) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {	
		GenericMapStore<BString, ProtocolInfo *>::Iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *info = it->second;
			
			UnloadInstance(info, false);
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

status_t ProtocolManager::MessageProtocols(ProtocolSpecification *match, BMessage *message, bool canDelete, bool appendSignature) {
	status_t result = B_ERROR;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		GenericMapStore<BString, ProtocolInfo *>::Iterator it;
		
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

//#pragma mark SpecificationFinder<ProtocolInfo *>

bool ProtocolManager::FindFirst(ProtocolSpecification *match, ProtocolInfo **firstMatch, bool canDelete) {
	bool found = false;
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		GenericMapStore<BString, ProtocolInfo *>::Iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *cur = it->second;
			
			if (match->IsSatisfiedBy(cur) == true) {
				*firstMatch = cur;
				found = true;
				break;
			};
		};
	};
	
	if (canDelete == true) delete match;
	
	return found;
};

GenericListStore<ProtocolInfo *> ProtocolManager::FindAll(ProtocolSpecification *match, bool canDelete) {
	GenericListStore<ProtocolInfo *> protocols(false);
	BAutolock lock(fLock);
	
	if (lock.IsLocked() == true) {
		GenericMapStore<BString, ProtocolInfo *>::Iterator it;
		
		for (it = fProtocol->Start(); it != fProtocol->End(); it++) {
			ProtocolInfo *info = it->second;
			
			if (match->IsSatisfiedBy(info) == true) {
				protocols.Add(info);
			};
		};
	};
	
	if (canDelete == true) delete match;
	
	return protocols;
};

//#pragma mark Private

status_t ProtocolManager::UnloadInstance(ProtocolInfo *info, bool remove) {
	if (info->HasValidMessenger() == true) {
		info->Messenger()->SendMessage(B_QUIT_REQUESTED);
	} else {
		// A half loaded addon - force it to exit
		kill_thread(info->ThreadID());
	};

	if (remove == true) {	
		fProtocol->Remove(info->InstanceID());
	};
	
	return B_OK;
};
