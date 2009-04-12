/*
 * Copyright 2009-, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#include <support/String.h>

typedef enum {
	Online,
	Away,
	Offline
} AccountStatus;

class AccountInfo {
	public:
						AccountInfo(const char *id, const char *name, const char *protocol, const char *friendlyProtocol, const char *status);
						~AccountInfo(void);
	
		// Public
		const char		*ID(void) const;
		const char		*Name(void) const;
		const char		*Protocol(void) const;
		const char		*FriendlyProtocol(void) const;
	
		const char		*StatusLabel(void) const;
		AccountStatus	Status(void) const;
		void			SetStatus(const char *status);
	
		const char		*DisplayLabel(void) const;
	
	private:
		BString			fID;
		BString			fName;
		BString			fProtocol;
		BString			fFriendlyProtocol;
		AccountStatus	fStatus;
};

#endif
