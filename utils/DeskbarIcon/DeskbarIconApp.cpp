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

#include "DeskbarIconUtils.h"
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
			BAlert* alert = new BAlert("", "Do you want IM Kit to live in "
				"the Deskbar?", "Don't", "Install", NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->SetShortcut(0, B_ESCAPE);

			if (alert->Go() == 1)
				_InstallInDeskbar();

			Quit();
		}

	private:
		void _InstallInDeskbar()
		{
			image_info info;
			entry_ref ref;

			if (our_image(info) == B_OK
				&& get_ref_for_path(info.name, &ref) == B_OK) {
				BDeskbar deskbar;
				deskbar.AddItem(&ref);
			}
		}
};

int
main(int argc, char* argv[])
{
	DeskbarIconApp app;
	app.Run();

	return 0;
}
