/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_INFO_H
#define _ACCOUNT_INFO_H

#include <support/String.h>

namespace IM {

typedef enum {
	Online = 0,
	Away,
	Offline
} AccountStatus;

class AccountInfo {
public:
					AccountInfo(const char* id, const char* protocol, const char* name,
						const char* friendlyProtocol, const char* status);
					~AccountInfo();

	const char*		ID() const;
	const char*		Name() const;
	const char*		Protocol() const;
	const char*		FriendlyProtocol() const;

	const char*		StatusLabel() const;
	AccountStatus	Status() const;
	void			SetStatus(const char* status);

	const char*		DisplayLabel() const;

private:
	BString			fID;
	BString			fName;
	BString			fProtocol;
	BString			fFriendlyProtocol;
	AccountStatus	fStatus;
};

};

#endif	// _ACCOUNT_INFO_H
