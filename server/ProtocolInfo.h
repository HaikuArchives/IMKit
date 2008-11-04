#ifndef PROTOCOLINFO_H
#define PROTOCOLINFO_H

#include <kernel/OS.h>
#include <support/String.h>
#include <storage/Path.h>

class BMessenger;

namespace IM {

	class ProtocolInfo {
		public:
							ProtocolInfo(BPath path, BPath settings);
							~ProtocolInfo(void);
						
			// Accessor Methods
			BPath			Path(void);
			BPath			SettingsPath(void);
			const char		*InstanceID(void);
			thread_id		ThreadID(void);

			const char		*Signature(void);
			void			Signature(const char *signature);
			const char		*FriendlySignature(void);
			void			FriendlySignature(const char *signature);		
			BMessenger		*Messenger(void);
			void			Messenger(BMessenger *messenger);
			uint32			Capabilities(void);
			void			Capabilities(uint32 caps);
			uint32			Encoding(void);
			void			Encoding(uint32 encoding);

			// Informational Methods
			bool			HasCapability(uint32 capability);
			bool			HasValidMessenger(void);
			bool			HasExited(void);

			// Control Methods
			status_t		Start(const char *loader);
			void			Stop(void);
			
			// Message Methods
			status_t		Process(BMessage *msg);
			status_t		UpdateSettings(BMessage *msg);
			
		private:	
			BString			fInstanceID;
			BPath			fPath;
			BPath			fSettingsPath;

			BString			fSignature;
			BString			fFriendlySignature;
			
			thread_id		fThreadID;
			BMessenger		*fMessenger;
			uint32			fCapabilities;
			uint32			fEncoding;
	};

};

#endif
