/*
 * Copyright 2008-2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */

#include <Message.h>
#include <Path.h>

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>
#include <libim/Connection.h>


int
main(int argc, char* argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "Usage: im_setstatus [option] [protocol] [account name]\n"
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
	if (argc == 4) {
		BString connString(argv[2]);
		connString << ":" << argv[3];

		IM::Connection connection(connString);
		newmsg.AddString("protocol", connection.Protocol());
		newmsg.AddString("id", connection.ID());
	}

	IM::Manager man;
	if (man.SendMessage(&newmsg) != B_OK) {
		fprintf(stderr, "Couldn't send message to IM Kit server, please "
			"check if it's running!\n");
		return 1;
	}

	return 0;
}
