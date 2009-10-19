#ifndef IMINFOAPP_H
#define IMINFOAPP_H

#include <Application.h>
#include <Entry.h>
#include <Directory.h>
#include <Bitmap.h>
#include <Path.h>

#include <map>

#include <libim/Contact.h>
#include <libim/Helpers.h>
#include <libim/Constants.h>
#include <libim/Manager.h>

#include <InfoPopper.h>
#include <common/IMKitUtilities.h>

typedef std::map<BString, entry_ref> protoicons;

class IMInfoApp : public BApplication {
	public:
					IMInfoApp(void);
					~IMInfoApp(void);
				
			void	MessageReceived(BMessage *msg);
			
	private:
		IM::Manager	*fManager;
		
		BString		fStatusText;
		BString		fMessageText;
//		int32		fIconSize;
		
		protoicons	fProtocolIcons;
};

#endif
