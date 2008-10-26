#include "AddApp.h"

#include <Roster.h>

#include <libim/Contact.h>
#include <libim/Constants.h>
#include <libim/Manager.h>
#include <libim/Helpers.h>

//#pragma mark Exported Hooks

extern "C" void process_refs(entry_ref dir_ref, BMessage *msg, void *) {
	msg->what = B_REFS_RECEIVED;
	msg->AddRef("dir_ref", &dir_ref);
	
	be_roster->Launch("application/x-vnd.BeClan.im_serverlist_add", msg);
};

// #pragma mark Functions

int main(void) {
	ServerListAddApp app;
	app.Run();
};

//#pragma mark Constructor

ServerListAddApp::ServerListAddApp(void)
	: BApplication("application/x-vnd.BeClan.im_serverlist_add") {
};

ServerListAddApp::~ServerListAddApp(void) {
};

//#pragma mark Hooks

void ServerListAddApp::RefsReceived(BMessage *msg) {
	entry_ref ref;
	BNode node;
	
	IM::Manager man;
	
	for (int32 i = 0; msg->FindRef("refs", i, &ref ) == B_OK; i++ ) {
		node = BNode(&ref);
		char *type = ReadAttribute(node, "BEOS:TYPE");
		if (strcmp(type, "application/x-person") == 0) {
			IM::Contact contact(ref);
			if (contact.CountConnections() == 0) continue;
			
			for (int32 i = 0; i < contact.CountGroups(); i++) {
				printf("Group %i: %s\n", i, contact.GroupAt(i));
			};
			
			BMessage add(IM::MESSAGE);
			add.AddInt32("im_what", IM::SERVER_LIST_ADD_CONTACT);
			add.AddRef("contact", &ref);
			
			add.PrintToStream();
			
			man.SendMessage(&add);
		};
		free(type);
	};
	
	BMessenger(this).SendMessage(B_QUIT_REQUESTED);
};
						
void ServerListAddApp::MessageReceived(BMessage *msg) {
	BApplication::MessageReceived(msg);
};

