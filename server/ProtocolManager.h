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
								
			// Loading / Unloading Protocols
			status_t			LoadFromDirectory(BDirectory protocols, BDirectory settings);
			status_t			ReloadProtocolFromDirectory(BDirectory protocols, BDirectory settings);
			status_t			RestartProtocols(ProtocolSpecification *match, bool canDelete = true);
			status_t			Unload(void);
			
			status_t			MessageProtocols(ProtocolSpecification *match, BMessage *message, bool canDelete = true, bool appendSignature = true);
			
			// SpecificationFinder<ProtocolInfo *> Methods
			ProtocolInfo		*FindFirst(ProtocolSpecification *match, bool deleteSpec = true);
			GenericListStore<ProtocolInfo *>
								FindAll(ProtocolSpecification *match, bool deleteSpec = true);
								
		private:
			BPath				fLoaderPath;
			ProtocolStore		*fProtocol;
			BLocker				*fLock;
	};
};

#endif
