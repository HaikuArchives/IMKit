/*
 * Copyright 2008-2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */

#include <Message.h>
#include <Path.h>

#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>


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
		// Get protocol path
		BPath protocolPath;
		if (im_protocol_get_path(argv[2], &protocolPath) != B_OK) {
			fprintf(stderr, "Error finding protocol \"%s\" path!\n", argv[2]);
			return 1;
		}

		// Get protocol settings path
		BPath protocolSettingsPath;
		if (im_protocol_get_settings_path(argv[2], &protocolSettingsPath) != B_OK) {
			fprintf(stderr, "Error finding protocol \"%s\" settings path!\n", argv[2]);
			return 1;
		}

		BString instance(protocolPath.Path());
		instance << "->" << protocolSettingsPath.Path() << "/" << argv[3]
			<< " (" << argv[3] << ")";

		newmsg.AddString("instance_id", instance.String());
	}

	IM::Manager man;
	man.SendMessage(&newmsg);

	return 0;
}
