#ifndef CONTACTSPECIFICATION_H
#define CONTACTSPECIFICATION_H

#include <storage/Entry.h>
#include <support/Debug.h>

#include <libim/Connection.h>
#include <libim/Contact.h>

#include "common/Specification.h"
#include "ContactCachedConnections.h"
#include "ContactHandle.h"
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

								ASSERT_WITH_MESSAGE(fConnection.HasAccount() == true, "Connection does not indicate an account - this indicates incomplete multi-account support");
							};
							
			virtual bool	IsSatisfiedBy(ContactCachedConnections *contact) {
								bool match = false;
								
								ConnectionStore *store = contact->CachedConnections();
								for (ConnectionStore::Iterator it = store->Start(); it != store->End(); it++) {
									Connection con = (*it);

									if (con == fConnection) {
										match = true;
										break;
									};
								};

								return match;
							};
							
		private:
			Connection		fConnection;
	};
	
	class ContactHandleSpecification : public ContactSpecification {
		public:
							ContactHandleSpecification(ContactHandle handle)
								: ContactSpecification(),
								fHandle(handle) {
							};
		
			virtual bool	IsSatisfiedBy(ContactCachedConnections *contact) {
								return (contact->Handle() == fHandle);
							};
		private:
			ContactHandle	fHandle;
	};
};

#endif // CONTACTSPECIFICATION_H

