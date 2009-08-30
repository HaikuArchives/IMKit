/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <libim/Constants.h>

#include "AccountInfo.h"

using namespace IM;


AccountInfo::AccountInfo(const char* id, const char* protocol,
	const char* name, const char* friendlyProtocol, const char* status)
	: fID(id),
	fName(name),
	fProtocol(protocol),
	fFriendlyProtocol(friendlyProtocol)
{
	SetStatus(status);
}


AccountInfo::~AccountInfo()
{
}


const char*
AccountInfo::ID() const
{
	return fID.String();
}


const char*
AccountInfo::Name() const
{
	return fName.String();
}


const char*
AccountInfo::Protocol() const
{
	return fProtocol.String();
}


const char*
AccountInfo::FriendlyProtocol() const
{
	return fFriendlyProtocol.String();
}


const char*
AccountInfo::StatusLabel() const
{
	const char* status = NULL;

	switch (fStatus) {
		case Online:
			status = ONLINE_TEXT;
			break;
		case Away:
			status = AWAY_TEXT;
			break;
		case Offline:
			status = OFFLINE_TEXT;
			break;
	}

	return status;
}


AccountStatus
AccountInfo::Status() const
{
	return fStatus;
}


void
AccountInfo::SetStatus(const char* status)
{
	if (strcmp(status, ONLINE_TEXT) == 0) {
		fStatus = Online;
	} else {
		if (strcmp(status, AWAY_TEXT) == 0)
			fStatus = Away;
		else
			fStatus = Offline;
	}
}


const char *
AccountInfo::DisplayLabel() const
{
	BString name = Name();
	bool hasName = (name.Length() > 0);

	if (hasName == true)
		name << " (";

	name << FriendlyProtocol();

	if (hasName == true)
		name << ")";

	return name.String();
}
