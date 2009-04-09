/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
 
#include "ConnectionStore.h"
#include "ContactCachedConnections.h"

#include "common/GenericStore.h"

using namespace IM;

//#pragma mark Constructor

ContactCachedConnections::ContactCachedConnections(entry_ref ref)
	: Contact(ref),
	fConnections(new ConnectionStore()) {

	ReloadConnections();
};

//#pragma mark Public		

void ContactCachedConnections::ReloadConnections(void) {
	char buffer[512];

	fConnections->Clear();
	
	for (int32 i = 0; i < CountConnections(); i++) {
		if (ConnectionAt(i, buffer) == B_OK) {
			IM::Connection con(buffer);
			fConnections->Add(con);
		};
	};
};

ConnectionStore *ContactCachedConnections::CachedConnections(void) {
	return fConnections;
};

//#pragma mark Operators

ContactCachedConnections::operator const entry_ref *(void) const {
	Contact *contact = (Contact *)this;
	return *contact;
};

bool ContactCachedConnections::operator == (const entry_ref & entry) const {
	return ((*(Contact *)this)) == entry;
};
