/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_CACHED_CONNECTIONS_H
#define _CONTACT_CACHED_CONNECTIONS_H
 
#include <Entry.h>
 
#include <libim/Contact.h>

#include "ContactHandle.h"
 
namespace IM {
	class ConnectionStore;

	class ContactCachedConnections : public Contact {
	public:
							ContactCachedConnections(const entry_ref& ref);
							ContactCachedConnections(const BEntry& entry);

		// Public
		void				ReloadConnections();
		ConnectionStore*	CachedConnections();
		ContactHandle		Handle() const;

		// Operators
							operator const entry_ref* () const;
		bool				operator==(const entry_ref& entry) const;
		bool				operator==(const ContactHandle& handle) const;

	private:
		ConnectionStore*	fConnections;
	};	
};

#endif	// _CONTACT_CACHED_CONNECTIONS_H
