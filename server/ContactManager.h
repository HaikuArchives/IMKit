/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_MANAGER_H
#define _CONTACT_MANAGER_H

#include <list>

#include <Looper.h>

#include <libim/Contact.h>

#include "ContactHandle.h"
#include "ContactSpecification.h"
#include "ContactListener.h"

#include <common/SpecificationFinder.h>

class BQuery;

class ContactStore;
class QueryStore;

namespace IM {
	class Connection;

	class ContactManager : public BLooper,
		public SpecificationFinder<ContactCachedConnections*> {
	public:
							ContactManager(ContactListener* listener);
		virtual				~ContactManager();

		// BLooper Hooks
		virtual void		MessageReceived(BMessage* msg);

		// SpecificationFinder<ContactCachedConnections*> Hooks
		bool				FindFirst(ContactSpecification* spec,
			ContactCachedConnections** firstMatch, bool canDelete = true);
		GenericListStore<ContactCachedConnections*>
							FindAll(ContactSpecification* spec, bool canDelete = true);

		// Public
		status_t			Init();
		ContactCachedConnections *
							CreateContact(Connection connection, const char* namebase);

	private:
		void				HandleContactUpdate(BMessage*);
		void				UpdateContactStatusAttribute(Contact&);
		void				GetContactsForProtocol(const char* protocol, BMessage* msg);

		void				ContactAdded(ContactHandle);
		void				ContactModified(ContactHandle);
		void				ContactMoved(ContactHandle from, ContactHandle to);
		void				ContactRemoved(ContactHandle);

		ContactListener*	fContactListener;
		ContactStore*		fContact;

		QueryStore*			fQuery;
	};
};

#endif	// _CONTACT_MANAGER_H
