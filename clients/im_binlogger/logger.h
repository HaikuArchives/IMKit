#ifndef IM_LOGGER_H
#define IM_LOGGER_H

#include <libim/Manager.h>

#include "../../common/IMKitUtilities.h"

#include <Application.h>
#include <String.h>
#include <File.h>
#include <Directory.h>

#include <stdio.h>
#include <stdlib.h>

class LoggerApp : public BApplication
{
	public:
						LoggerApp();
						~LoggerApp();
			void		MessageReceived(BMessage *msg);
		
	private:
		IM::Manager		*fMan;
		BString			fLogParent;
};

#endif
