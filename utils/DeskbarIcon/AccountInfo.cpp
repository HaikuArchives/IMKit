#include "AccountInfo.h"

#include <libim/Constants.h>

//#pragma mark Constructor

AccountInfo::AccountInfo(const char *id, const char *name, const char *protocol, const char *friendlyProtocol, const char *status)
	: fID(id),
	fName(name),
	fProtocol(protocol),
	fFriendlyProtocol(friendlyProtocol) {
	
	SetStatus(status);
};

AccountInfo::~AccountInfo(void) {
};

//#pragma mark Public

const char *AccountInfo::ID(void) const {
	return fID.String();
};

const char *AccountInfo::Name(void) const {
	return fName.String();
};

const char *AccountInfo::Protocol(void) const {
	return fProtocol.String();
};

const char *AccountInfo::FriendlyProtocol(void) const {
	return fFriendlyProtocol.String();
};

const char *AccountInfo::StatusLabel(void) const {
	const char *status = NULL;
	switch (fStatus) {
		case Online: {
			status = ONLINE_TEXT;
		} break;
		case Away: {
			status = AWAY_TEXT;
		} break;
		case Offline: {
			status = OFFLINE_TEXT;
		} break;
	};
	
	return status;
};

AccountStatus AccountInfo::Status(void) const {
	return fStatus;
};

void AccountInfo::SetStatus(const char *status) {
	if (strcmp(status, ONLINE_TEXT) == 0) {
		fStatus = Online;
	} else {
		if (strcmp(status, AWAY_TEXT) == 0) {
			fStatus = Away;
		} else {
			fStatus = Offline;
		};
	};
};

const char *AccountInfo::DisplayLabel(void) const {
	BString name = Name();
	bool hasName = (name.Length() > 0);
	if (hasName == true) {
		name << " (";
	};
	name << FriendlyProtocol();
	if (hasName == true) {
		name << ")";
	};

	return name.String();
};
