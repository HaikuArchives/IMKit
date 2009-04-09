#ifndef CONTACTSPECIFICATION_H
#define CONTACTSPECIFICATION_H

#include <storage/Entry.h>

#include <libim/Contact.h>

#include "common/Specification.h"
#include "ContactCachedConnections.h"
#include "ConnectionStore.h"

namespace IM {
	typedef Specification<ContactCachedConnections *> ContactSpecification;
	
	class AllContactSpecification : public ContactSpecification {
		public:
			virtual bool	IsSatisfiedBy(ContactCachedConnections *contact) {
								return true;
							};
	};
	
	class EntryRefContactSpecification : public ContactSpecification {
		public:
							EntryRefContactSpecification(entry_ref ref)
								: ContactSpecification(),
								fRef(ref) {
							};
		
			virtual bool	IsSatisfiedBy(ContactCachedConnections *contact) {
								return (*contact == fRef);
							};
							
		private:
			entry_ref		fRef;
	};
	

	class ConnectionContactSpecification : public ContactSpecification {
		public:
							ConnectionContactSpecification(Connection connection)
								: ContactSpecification(),
								fConnection(connection) {
							};
							
			virtual bool	IsSatisfiedBy(ContactCachedConnections *contact) {
								bool match = false;
								
								ConnectionStore *store = contact->CachedConnections();
								for (ConnectionStore::Iterator it = store->Start(); it != store->End(); it++) {
									if ((*it) == fConnection) {
										match = true;
										break;
									};
								};

								return match;
							};
							
		private:
			Connection		fConnection;
	};
};

#endif // CONTACTSPECIFICATION_H

