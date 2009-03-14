#ifndef PROTOCOLINFO_H
#define PROTOCOLINFO_H

#include <app/Message.h>
#include <kernel/OS.h>
#include <support/String.h>
#include <storage/Path.h>

class BMessenger;

namespace IM {

	class ProtocolInfo {
		public:
							ProtocolInfo(BPath path, BPath settings, const char *account);
							~ProtocolInfo(void);
						
			// Accessor Methods
			BPath			Path(void);
			BPath			SettingsPath(void);
			const char		*InstanceID(void);
			thread_id		ThreadID(void);
			const char		*AccountName(void);

			const char		*Signature(void);
			void			Signature(const char *signature);
			const char		*FriendlySignature(void);
			void			FriendlySignature(const char *signature);		
			uint32			Capabilities(void);
			void			Capabilities(uint32 caps);
			uint32			Encoding(void);
			void			Encoding(uint32 encoding);
			BMessage		SettingsTemplate(void);
			void			SettingsTemplate(BMessage settings);

			BMessenger		*Messenger(void);
			void			Messenger(BMessenger *messenger);

			// Informational Methods
			bool			HasCapability(uint32 capability);
			bool			HasValidMessenger(void);
			bool			HasExited(void);
			bool			IsRestartAllowed(void);

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
			BString			fAccountName;

			BString			fSignature;
			BString			fFriendlySignature;
			uint32			fCapabilities;
			uint32			fEncoding;
			BMessage		fSettingsTemplate;
			
			thread_id		fThreadID;
			BMessenger		*fMessenger;
			bool			fAllowRestart;
	};

};

#endif
