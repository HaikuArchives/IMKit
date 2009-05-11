#include "im_server.h"

#include <interface/Alert.h>
#include <kernel/fs_index.h>
#include <support/Beep.h>
#include <storage/VolumeRoster.h>

#include <signal.h>
#include <stdio.h>

#include <libim/Helpers.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

//#pragma mark Installation / Setup Methods

void RegisterSoundEvents(void) {
	// protocol status
	add_system_beep_event(kImConnectedSound, 0);
	add_system_beep_event(kImDisconnectedSound, 0);
	
	// contact status
	add_system_beep_event(kImStatusOnlineSound, 0);
	add_system_beep_event(kImStatusAwaySound, 0);
	add_system_beep_event(kImStatusOfflineSound, 0);
};

void CheckIndexes(void) {
	BVolumeRoster roster;
	BVolume volume;
	int madeIndex = false;
	roster.Rewind();

	while (roster.GetNextVolume(&volume) == B_NO_ERROR) {
		char volName[B_OS_NAME_LENGTH];
		volume.GetName(volName);

		if ((volume.KnowsAttr()) && (volume.KnowsQuery()) && (volume.KnowsMime())) {
			DIR *indexes;
			struct dirent *ent;

			LOG(kAppName, liDebug, "%s is suitable for indexing", volName);

			indexes = fs_open_index_dir(volume.Device());
			if (indexes == NULL) {
				LOG(kAppName, liLow, "Error opening indexes on %s", volName);
				continue;
			};

			bool isConnIndexed = false;
			bool isStatusIndexed = false;

			while ( (ent = fs_read_index_dir(indexes)) != NULL ) {
				if (strcmp(ent->d_name, "IM:connections") == 0) isConnIndexed = true;
				if (strcmp(ent->d_name, "IM:status") == 0) isStatusIndexed = true;
			};

			if (isConnIndexed == false) {
				int res = fs_create_index(volume.Device(), "IM:connections", B_STRING_TYPE, 0);
				LOG(kAppName, liMedium, "Added IM:connections to %s: %s (%i)",
					volName, strerror(res), res);
				madeIndex = true;
			};

			if (isStatusIndexed == false) {
				int res = fs_create_index(volume.Device(), "IM:status", B_STRING_TYPE, 0);
				LOG(kAppName, liMedium, "Added IM:status to %s: %s (%i)",
					volName, strerror(res), res);
				madeIndex = true;
			};

			fs_close_index_dir(indexes);
		};
	};

	if (madeIndex) {
#ifdef __HAIKU__
		const char *msg = _T("The IM Kit had to add indexes for "
			"IM:connections or IM:status to one or more of your drives. Please be "
			"sure to re-index any People files on these drives, failure to do so "
			"may cause duplicate People files to be created.");
#else
		const char *msg = _T("The IM Kit had to add indexes for "
			"IM:connections or IM:status to one or more of your drives. Please be "
			"sure to re-index any People files on these drives. You should obtain "
			"reindex from http://www.bebits.com/app/2033 Failure to do so may cause "
			"duplicate People files to be created.");
#endif
		BAlert *alert = new BAlert(_T("The IM Kit"), msg, _T("Quit"), _T("OK"), NULL,
			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_IDEA_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		int32 choice = alert->Go();

		if (choice == 0)
			exit(1);
	};
}

//#pragma mark Signal Handlers

void handle_ctrl_c( int /*sig*/ ) {
	printf("Fatal signal received.\n");
	static bool has_sent_quit = false;
	
	if (!has_sent_quit) {
		printf("Quiting im_server.\n");
		BMessenger(be_app).SendMessage( B_QUIT_REQUESTED );
		has_sent_quit = true;
	};
};

//#pragma mark Main

int main(int numarg, const char ** argv) {
	RegisterSoundEvents();

	// add ctrl-c handler
	struct sigaction my_sig_action;
	my_sig_action.sa_handler = handle_ctrl_c;
	my_sig_action.sa_mask = 0;
	my_sig_action.sa_flags = 0;
	my_sig_action.sa_userdata = 0;
	
	// not sure exactly which signals we want to monitor
	// better one too many than one too few?
	sigaction( SIGHUP, &my_sig_action, NULL );
	sigaction( SIGINT, &my_sig_action, NULL );
	sigaction( SIGQUIT, &my_sig_action, NULL );
	//sigaction( SIGTERM, &my_sig_action, NULL );
	
	// check commandline args
	int curr = 1;

	while ( curr < numarg )
	{
		if ( strcmp(argv[curr], "-v") == 0 )
		{ // verbosity level
			if ( curr == numarg - 1 )
			{ // no v-level provided
				printf("No verbosity level provided!\n");
				return 1;
			}
			if ( strcmp(argv[curr+1], "debug") == 0 )
			{
				g_verbosity_level = liDebug;
			} else
			if ( strcmp(argv[curr+1], "high") == 0 )
			{
				g_verbosity_level = liHigh;
			} else
			if ( strcmp(argv[curr+1], "medium") == 0 )
			{
				g_verbosity_level = liMedium;
			} else
			if ( strcmp(argv[curr+1], "low") == 0 )
			{
				g_verbosity_level = liLow;
			} else
			if ( strcmp(argv[curr+1], "quiet") == 0 )
			{
				g_verbosity_level = liQuiet;
			} else
			{ // unknown v-level
				printf("Unknown verbosity level\n");
				return 1;
			}
			curr++;
		} else
		if ( strcmp(argv[curr], "-h") == 0 )
		{ // help
			printf("usage: im_server [options].\n");
			printf("options:\n");
			printf("   -h        Display this help\n");
			printf("   -v LEVEL  Set the output verbosity level.\n");
			printf("             LEVEL is one of (debug, high, medium,\n");
			printf("             low, quiet). Default is medium.\n");
			return 0;
		} else
		{ // unknown options, exit with error
			printf("unrecognized options: %s\n", argv[curr]);
			printf("run im_server -h for more info on options\n");
			return 1;
		}
		
		curr++;
	}
	
	// rotate log file
	rename("/boot/home/im_kit.log", "/boot/home/im_kit.log.0");

	IM::Server server;
	CheckIndexes();
	server.Run();

	return 0;
};
