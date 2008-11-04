#ifndef PROTOCOL_LOADER_APPLICATION_H
#define PROTOCOL_LOADER_APPLICATION_H

#include <Application.h>
#include <String.h>

namespace IM {
	class Manager;
	class Protocol;
};

using namespace IM;

class ProtocolLoaderApplication : public BApplication {
	public:
								ProtocolLoaderApplication(const char *instanceID,
									Protocol *protocol, BMessage settings,
									status_t *error);
								~ProtocolLoaderApplication(void);
	
		// BApplication Hooks
		void					ReadyToRun(void);
		bool					QuitRequested(void);
		void					MessageReceived(BMessage *msg);
	
	private:
		BString					fInstanceID;
		Protocol				*fProtocol;
		BMessage				fSettings;
		
		Manager					*fManager;
};

#endif
