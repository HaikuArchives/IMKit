#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <storage/Directory.h>
#include <storage/Path.h>
#include <support/String.h>

#include <list>

#include "ProtocolSpecification.h"
#include "common/SpecificationFinder.h"

class BLocker;
class BMessage;

namespace IM {

	class ProtocolInfo;	
	class ProtocolStore;

	class ProtocolManager : public SpecificationFinder<ProtocolInfo *> {
		public:
								ProtocolManager(void);
			virtual				~ProtocolManager(void);

			status_t		InitCheck();
								
			// Loading / Unloading Protocols
			ProtocolInfo		*LaunchInstance(BPath protocolPath, BPath settingsPath, const char *name);
			status_t			LoadFromDirectory(BDirectory protocols, BDirectory settings);
			status_t			RestartProtocols(ProtocolSpecification *match, bool canDelete = true);
			status_t			UnloadInstance(ProtocolInfo *info);
			status_t			Unload(void);
			
			status_t			MessageProtocols(ProtocolSpecification *match, BMessage *message, bool canDelete = true, bool appendSignature = true);
			
			// SpecificationFinder<ProtocolInfo *> Methods
			bool				FindFirst(ProtocolSpecification *match, ProtocolInfo **firstMatch, bool deleteSpec = true);
			GenericListStore<ProtocolInfo *>
								FindAll(ProtocolSpecification *match, bool deleteSpec = true);
								
		private:
			status_t			UnloadInstance(ProtocolInfo *info, bool remove);
		
			BPath				fLoaderPath;
			ProtocolStore		*fProtocol;
			BLocker				*fLock;
	};
};

#endif
