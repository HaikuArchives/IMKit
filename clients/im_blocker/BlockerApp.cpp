#include "BlockerApp.h"

#include <Roster.h>

#include <libim/Contact.h>
#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>

//#pragma mark Exported Hooks

extern "C" void process_refs(entry_ref dir_ref, BMessage *msg, void *) {
	msg->what = B_REFS_RECEIVED;
	msg->AddRef("dir_ref", &dir_ref);
	
	be_roster->Launch("application/x-vnd.BeClan.im_blocker", msg);
};

// #pragma mark Functions

int main(void) {
	BlockerApp app;
	app.Run();
};

//#pragma mark Constructor

BlockerApp::BlockerApp(void)
	: BApplication("application/x-vnd.BeClan.im_blocker") {
};

BlockerApp::~BlockerApp(void) {
};

//#pragma mark Hooks

void BlockerApp::RefsReceived(BMessage *msg) {
	entry_ref ref;
	BNode node;
	attr_info info;
	
	IM::Manager man;
	
	for (int32 i = 0; msg->FindRef("refs", i, &ref ) == B_OK; i++ ) {
		node = BNode(&ref);
		char *type = ReadAttribute(node, "BEOS:TYPE");
		if (strcmp(type, "application/x-person") == 0) {

			IM::Contact contact(ref);
			
			char status[256];
			
			if (contact.GetStatus(status, sizeof(status)) != B_OK )	status[0] = '\0';
			
			if (strcmp(status, BLOCKED_TEXT) == 0) {
				// already blocked, unblocked
				contact.SetStatus(OFFLINE_TEXT);
				
				BMessage update_msg(IM::UPDATE_CONTACT_STATUS);
				update_msg.AddRef("contact", &ref);
				
				man.SendMessage(&update_msg);
			} else {
				if (contact.SetStatus(BLOCKED_TEXT) != B_OK) {
					LOG("im_blocker", liHigh, "Error setting contact status");
				};
			};
		};
		free(type);
	};
	
	BMessenger(this).SendMessage(B_QUIT_REQUESTED);
};
						
void BlockerApp::MessageReceived(BMessage *msg) {
	BApplication::MessageReceived(msg);
};

