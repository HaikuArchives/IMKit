#include <Application.h>
#include <Entry.h>

#include "PeopleWindow.h"

class PeopleApp : public BApplication {
	public:
		PeopleApp();
		
		void RefsReceived(BMessage *);
		void ReadyToRun();
		
	private:
		bool got_a_person;
};

int main(void) {
	PeopleApp().Run();
	return 0;
}

PeopleApp::PeopleApp() : BApplication("application/x-vnd.Be-PEPL"), got_a_person(false) {}

void PeopleApp::RefsReceived(BMessage *msg) {
	entry_ref ref;
	
	got_a_person = true;
	for (int32 i = 0; msg->FindRef("refs",i,&ref) == B_OK; i++)
		(new PeopleWindow(&ref))->Show();
}

void PeopleApp::ReadyToRun() {
	if (!got_a_person)
		(new PeopleWindow(NULL))->Show();
}
