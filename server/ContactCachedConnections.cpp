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

ContactCachedConnections::ContactCachedConnections(const entry_ref &ref)
	: Contact(ref),
	fConnections(new ConnectionStore()) {

	ReloadConnections();
};

ContactCachedConnections::ContactCachedConnections(const BEntry &entry)
	: Contact(entry),
	fConnections(new ConnectionStore()) {
	
	ReloadConnections();
};

//#pragma mark Public

void ContactCachedConnections::ReloadConnections(void) {
	char buffer[512];

	fConnections->Clear();
	Update();
	
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

ContactHandle ContactCachedConnections::Handle(void) const {
	ContactHandle us;
	us.entry = EntryRef();

	node_ref nref;
	BNode node(&us.entry);
	node.GetNodeRef(&nref);
	
	us.node = nref.node;

	return us;
};

//#pragma mark Operators

ContactCachedConnections::operator const entry_ref *(void) const {
	Contact *contact = (Contact *)this;
	return *contact;
};

bool ContactCachedConnections::operator == (const entry_ref & entry) const {
	return ((*(Contact *)this)) == entry;
};

bool ContactCachedConnections::operator == (const ContactHandle &handle) const {
	ContactHandle us = Handle();	
	return (handle == us);	
};
