#include "QueryApp.h"

QueryApplication::QueryApplication(void)
	: BApplication("application/x-vnd.BeClan.QueryViewer") {
};

QueryApplication::~QueryApplication(void) {
};

bool QueryApplication::QuitRequested(void) {
	return BApplication::QuitRequested();
};
						
void QueryApplication::ReadyToRun(void) {
	fWindow = new QueryWindow(BRect(30, 30, 500, 200));
};

void QueryApplication::MessageReceived(BMessage *msg) {
	BApplication::MessageReceived(msg);
};

