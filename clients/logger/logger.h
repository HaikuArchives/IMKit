#ifndef IM_LOGGER_H
#define IM_LOGGER_H

#include <libim/Manager.h>

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

/*		
		status_t		SetMaxFiles(int max);
		int				MaxFiles(void);
*/
		
		void MessageReceived( BMessage * );
		
	private:
		IM::Manager *	fMan;

//		Eventually we should implement some sort of file cache where the last
//		fMaxFiles are kept open
/*	
		int				fMaxFiles;
		BFile			**fFiles;
*/
};

#endif
