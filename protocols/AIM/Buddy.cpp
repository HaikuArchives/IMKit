#include "Buddy.h"

Buddy::Buddy(void) {
	fGroupID = 0;
	fItemID = 0;
	fName = "";
};

Buddy::Buddy(const char *name, uint16 group, uint16 item) {
	fName = name;
	fGroupID = group;
	fItemID = item;
};

uint16 Buddy::GroupID(void) {
	return fGroupID;
};

uint16 Buddy::ItemID(void) {
	return fItemID;
};

const char *Buddy::Name(void) {
	return fName.String();
};

