#include <Application.h>
#include <Entry.h>
#include <E-mail.h>
#include <Roster.h>

#include "IMKitUtilities.h"

const char kAppSig[] = "application/x-vnd.BeClan.IMKit.MailSetter";

extern "C" void process_refs(entry_ref dir_ref, BMessage *msg, void *) {
	msg->what = B_REFS_RECEIVED;
	msg->AddRef("dir_ref", &dir_ref);
	
	be_roster->Launch(kAppSig, msg);
};

class MSApp : public BApplication {
	public:
				MSApp(void)
					: BApplication(kAppSig) {
				};
	
		void	RefsReceived(BMessage *msg) {
					entry_ref ref;

					for (int32 i = 0; msg->FindRef("refs", i, &ref) == B_OK; i++) {
						BNode node(&ref);
						node.WriteAttr(B_MAIL_ATTR_STATUS, B_STRING_TYPE, 0, "Read",
							strlen("Read"));
					};
					
					be_app_messenger.SendMessage(B_QUIT_REQUESTED);
				};
};

//#pragma mark -

int main(int argc, char *argv[]) {
	MSApp app;
	app.Run();
};
