/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef CONTACT_MANAGER_H
#define CONTACT_MANAGER_H

#include <app/Looper.h>

#include <list>

#include <libim/Contact.h>

#include "ContactHandle.h"
#include "ContactSpecification.h"
#include "common/GenericStore.h"
#include "common/SpecificationFinder.h"

class BQuery;

class ContactStore;
class QueryStore;

namespace IM {
	class ContactListener;

	class ContactManager : public BLooper, public SpecificationFinder<ContactCachedConnections *> {
		public:
								ContactManager(ContactListener *listener);
			virtual				~ContactManager(void);
	
			// BLooper Hooks
			virtual void		MessageReceived(BMessage *msg);
			
			// SpecificationFinder<Contact *> Hooks
			ContactCachedConnections
								*FindFirst(ContactSpecification *spec, bool canDelete = true);
			GenericListStore<ContactCachedConnections *>
								FindAll(ContactSpecification *spec, bool canDelete = true);

			// Public
			status_t			Init(void);		
			ContactHandle		CreateContact(const char * proto_id, const char *namebase);
	
		private:
			void				HandleContactUpdate(BMessage *);
			void				UpdateContactStatusAttribute(Contact &);
			void				GetContactsForProtocol(const char * protocol, BMessage * msg);
	
			void				ContactAdded(ContactHandle);
			void				ContactModified(ContactHandle);
			void				ContactMoved(ContactHandle from, ContactHandle to);
			void				ContactRemoved(ContactHandle);
	
			
			ContactListener		*fContactListener;
			ContactStore		*fContact;
			
			QueryStore			*fQuery;
	};
};

#endif // CONTACT_MANAGER_H
