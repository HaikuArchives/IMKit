/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <app/Application.h>
#	include <interface/Deskbar.h>
#include <interface/Alert.h>
#include <storage/Entry.h>

#include <common/IMKitUtilities.h>

#include "DeskbarIconResources.h"

class DeskbarIconApp : public BApplication
{
	public:
		DeskbarIconApp()
			: BApplication(DESKBAR_ICON_SIG)
		{
		}

		void ReadyToRun()
		{
		}

	private:
		void _InstallInDeskbar()
		{
		}
};

int
main(int argc, char* argv[])
{
	DeskbarIconApp app;
	app.Run();

	return 0;
}
