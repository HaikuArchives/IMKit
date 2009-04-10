#include "im_server.h"

#include <signal.h>
#include <stdio.h>

#include <libim/Helpers.h>

void handle_ctrl_c( int /*sig*/ ) {
	printf("Fatal signal received.\n");
	static bool has_sent_quit = false;
	
	if (!has_sent_quit) {
		printf("Quiting im_server.\n");
		BMessenger(be_app).SendMessage( B_QUIT_REQUESTED );
		has_sent_quit = true;
	};
};

int main(int numarg, const char ** argv) {
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
};
