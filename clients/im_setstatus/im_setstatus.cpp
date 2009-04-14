/*
 * Copyright 2008, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */

#include <app/Message.h>

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>

int
main(int argc, char* argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "Usage: im_setstatus [OPTION]\n"
			"\t--online     Set all the protocols online\n"
			"\t--offline    Set all the protocols offline\n"
			"\t--away       Set all the protocols away\n");
		return 1;
	}

	BMessage newmsg(IM::MESSAGE);
	newmsg.AddInt32("im_what", IM::SET_STATUS);
	if (strcmp(argv[1], "--online") == 0)
		newmsg.AddString("status", ONLINE_TEXT);
	else if (strcmp(argv[1], "--offline") == 0)
		newmsg.AddString("status", OFFLINE_TEXT);
	else if (strcmp(argv[1], "--away") == 0)
		newmsg.AddString("status", AWAY_TEXT);

	IM::Manager man;
	man.SendMessage(&newmsg);

	return 0;
}
