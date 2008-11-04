#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <storage/Directory.h>
#include <storage/Path.h>
#include <support/String.h>

#include <list>

class BLocker;
class BMessage;

namespace IM {

	class ProtocolInfo;	
	class ProtocolSpecification;
	class ProtocolStore;

	class ProtocolManager {
		public:
								ProtocolManager(void);
								~ProtocolManager(void);
								
			// Loading / Unloading Protocols
			status_t			LoadFromDirectory(BDirectory protocols, BDirectory settings);
			status_t			RestartProtocols(ProtocolSpecification *match, bool canDelete = true);
			status_t			Unload(void);
			
			status_t			MessageProtocols(ProtocolSpecification *match, BMessage *message, bool canDelete = true, bool appendSignature = true);
			ProtocolInfo		*FindProtocol(ProtocolSpecification *match, bool canDelete = true);
			list<ProtocolInfo *>FindProtocols(ProtocolSpecification *match, bool canDelete = true);
								
		private:
			BPath				fLoaderPath;
			ProtocolStore		*fProtocol;
			BLocker				*fLock;
	};
};

#endif
