/*
 * Copyright 2008-, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef PROTOCOL_LOADER_APPLICATION_H
#define PROTOCOL_LOADER_APPLICATION_H

#include <Application.h>
#include <String.h>

namespace IM {
	class Manager;
	class Protocol;

	class ProtocolLoaderApplication : public BApplication {
		public:
									ProtocolLoaderApplication(const char *instanceID, Protocol *protocol, const char* path, BMessage settings, const char *accountName);
									~ProtocolLoaderApplication(void);
		
			// BApplication Hooks
			void					ReadyToRun(void);
			bool					QuitRequested(void);
			void					MessageReceived(BMessage *msg);
		
		private:
			BString					fInstanceID;
			Protocol				*fProtocol;
			BString					fProtoName;
			BMessage				fSettings;
			BString					fAccountName;
			
			Manager					*fManager;
	};
};

#endif // PROTOCOL_LOADER_APPLICATION_H
