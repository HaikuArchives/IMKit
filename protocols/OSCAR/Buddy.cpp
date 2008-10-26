#include "Buddy.h"

#include <algorithm>

#include "OSCARConstants.h"
#include "BufferReader.h"

//#pragma mark Constructor

Buddy::Buddy(void) {
	fItemID = 0;
	fName = "";
};

Buddy::Buddy(const char *name, uint16 item) {
	fName = name;
	fItemID = item;
};

Buddy::~Buddy(void) {
	ClearCapabilities();
};

//#pragma mark Public Methods

uint16 Buddy::ItemID(void) {
	return fItemID;
};

const char *Buddy::Name(void) {
	return fName.String();
};

int32 Buddy::CountGroups(void) {
	return fGroups.size();
};

uint16 Buddy::GroupAt(int32 index) {
	return fGroups[index];
};

void Buddy::AddGroup(uint16 group) {
	fGroups.push_back(group);
};

void Buddy::ClearGroups(void) {
	fGroups.clear();
};

bool Buddy::IsInGroup(uint16 group) {
	groupid_t::iterator gIt = find(fGroups.begin(), fGroups.end(), group);
	return gIt != fGroups.end();
};

void Buddy::SetUserclass(uint16 userclass) {
	fUserclass = userclass;
};

uint16 Buddy::Userclass(void) {
	return fUserclass;
};

bool Buddy::HasCapability(const char *capability, int32 len) {
	BufferReader cap((const uchar *)capability, len);
	int32 caps = fCapabilities.size();
	bool hasCap = false;
	
	for (int32 i = 0; i < caps; i++) {
		if (cap == fCapabilities[i]) {
			hasCap = true;
			break;
		};
	};
	
	return hasCap;
};

void Buddy::AddCapability(const char *capability, int32 len) {
	uchar *buffer = (uchar *)calloc(len, sizeof(char));
	memcpy(buffer, capability, len);

	fCapabilities.push_back(new BufferReader(buffer, len));
};

void Buddy::ClearCapabilities(void) {
	int32 caps = fCapabilities.size();
	for (int32 i = 0; i < caps; i++) delete fCapabilities[i];

	fCapabilities.clear();
};

bool Buddy::IsMobileUser(void) {
	bool mobileUser = (fUserclass & CLASS_WIRELESS) == CLASS_WIRELESS;	
	char hiptop[] = {0x09, 0x46, 0x13, 0x23, 0x4c, 0x7f, 0x11, 0xd1, 
		  0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
	
	bool isHiptop = HasCapability(hiptop, sizeof(hiptop));

	return mobileUser || isHiptop;
};
