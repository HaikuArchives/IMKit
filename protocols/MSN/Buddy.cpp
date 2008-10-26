#include "Buddy.h"

Buddy::Buddy(const char *passport) {
	fPassport = passport;
	fFriendly = "";
	fStatus = otOffline;
	fCaps = ccMultiPacketMessaging;
	fLists = ltNone;
	fDisplay = NULL;
};

Buddy::~Buddy() {
	if (fDisplay) delete fDisplay;
};

const char *Buddy::Passport(void) {
	return fPassport.String();
};

void Buddy::Passport(const char *passport) {
	fPassport = passport;
};

const char *Buddy::FriendlyName(void) {
	return fFriendly.String();
};

void Buddy::FriendlyName(const char *friendly) {
	fFriendly = friendly;
};

int8 Buddy::Status(void) {
	return fStatus;
};

void Buddy::Status(int8 status) {
	fStatus = status;
};

int32 Buddy::Capabilities(void) {
	return fCaps;
};

void Buddy::Capabilities(int32 caps) {
	fCaps = caps;
};

MSNObject *Buddy::DisplayPicture(void) {
	return fDisplay;
};

void Buddy::DisplayPicture(MSNObject *obj) {
	if (fDisplay) delete fDisplay;
	fDisplay = obj;
};

int32 Buddy::Lists(void) {
	return fLists;
};

void Buddy::Lists(int32 lists) {
	fLists = lists;
};
	
