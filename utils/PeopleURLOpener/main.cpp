#include <Application.h>
#include <Entry.h>
#include <Roster.h>

#include "IMKitUtilities.h"

extern "C" void process_refs(entry_ref dir_ref, BMessage *msg, void *) {
	msg->what = B_REFS_RECEIVED;
	msg->AddRef("dir_ref", &dir_ref);
	
	be_roster->Launch("application/x-vnd.BeClan.IMKit.PeopleURLOpener", msg);
};

class PUPApp : public BApplication {
	public:
				PUPApp(void)
					: BApplication("application/x-vnd.BeClan.IMKit.PeopleURLOpener") {
				};
	
		void	RefsReceived(BMessage *msg) {
					entry_ref ref;
					int32 urlsAdded = 0;

					entry_ref htmlRef;
					be_roster->FindApp("application/x-vnd.Be.URL.http", &htmlRef);
					BPath htmlPath(&htmlRef);

					BMessage argv(B_ARGV_RECEIVED);
					argv.AddString("argv", htmlPath.Path());
					
					for (int32 i = 0; msg->FindRef("refs", i, &ref) == B_OK; i++) {						
						BPath path(&ref);
						int32 length = -1;
						char *url = ReadAttribute(path.Path(), "META:url", &length);
						if ((url != NULL) && (length > 1)) {
							url = (char *)realloc(url, (length + 1) * sizeof(char));
							url[length] = '\0';
							argv.AddString("argv", url);
							
							urlsAdded++;
						};
						if (url) free(url);
					};
					
					if (urlsAdded > 0) {
						argv.AddInt32("argc", urlsAdded + 1);
						be_roster->Launch(&htmlRef, &argv);
					};
					
					be_app_messenger.SendMessage(B_QUIT_REQUESTED);
				};
};

//#pragma mark -

int main(int argc, char *argv[]) {
	PUPApp app;
	app.Run();
};
