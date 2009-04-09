#include "ContactManager.h"

#include <storage/Entry.h>
#include <storage/Query.h>
#include <storage/VolumeRoster.h>
#include <storage/NodeMonitor.h>
#include <support/Autolock.h>
#include <support/Locker.h>

#include <libim/Contact.h>
#include <libim/Helpers.h>

#include "ContactCachedConnections.h"
#include "ContactListener.h"
#include "common/SpecificationFinder.h"

//#pragma mark Definitions

using namespace IM;

class QueryStore : public GenericListStore<BQuery *> {
};

class ContactStore : public GenericMapStore<entry_ref, ContactCachedConnections *>,  public SpecificationFinder<ContactCachedConnections *> {
	public:
		ContactStore(void)
			: GenericMapStore<entry_ref, ContactCachedConnections *>(),
			fLock(new BLocker()) {
		};
		
		virtual ~ContactStore(void) {
			delete fLock;
		};
	
		virtual ContactCachedConnections *FindFirst(ContactSpecification *specification, bool deleteSpec = true) {
			ContactCachedConnections *contact = NULL;
			BAutolock lock(fLock);
			
			if (lock.IsLocked() == true) {
				GenericMapStore<entry_ref, ContactCachedConnections *>::Iterator it;
				for (it = Start(); it != End(); it++) {
					ContactCachedConnections *current = it->second;
					
					if (specification->IsSatisfiedBy(current) == true) {
						contact = current;
						break;
					};
				};
			};
			
			if (deleteSpec == true) {
				delete specification;
			};
			
			return contact;
		};

		virtual GenericListStore<ContactCachedConnections *> FindAll(ContactSpecification *specification, bool deleteSpec = true) {
			GenericListStore<ContactCachedConnections *> contacts(false);
			BAutolock lock(fLock);
			
			if (lock.IsLocked() == true) {
				GenericMapStore<entry_ref, ContactCachedConnections *>::Iterator it;
				for (it = Start(); it != End(); it++) {
					ContactCachedConnections *current = it->second;
					
					if (specification->IsSatisfiedBy(current) == true) {
						contacts.Add(current);
					};
				};
			};
			
			if (deleteSpec == true) {
				delete specification;
			};
			
			return contacts;
		};
	
	private:
		BLocker	*fLock;
};

//#pragma mark Constructors

extern char *kAppName;

//#pragma mark Constructor

ContactManager::ContactManager(ContactListener *listener)
	: BLooper("ContactManager", B_LOW_PRIORITY),
	fContactListener(listener),
	fContact(new ContactStore()), 
	fQuery(new QueryStore()) {
};

ContactManager::~ContactManager(void) {
	delete fQuery;
};

//#pragma mark BLooper Hooks

void ContactManager::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		// Contact messages. Both query and node monitor.
		case B_QUERY_UPDATE: {
			int32 opcode = B_ERROR;
			
			if (msg->FindInt32("opcode", &opcode) != B_OK) {
				return;
			};
			
			ContactHandle handle;
			const char *name = NULL;
			
			switch (opcode) {
				case B_ENTRY_CREATED: {
					// .. add to contact list
					msg->FindString("name", &name );
					handle.entry.set_name(name);
					msg->FindInt64("directory", &handle.entry.directory);
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);
					
					ContactAdded(handle);
				} break;
				
				default: {
				} break;
			}
		} break;
		
		case B_NODE_MONITOR: {
			int32 opcode = 0;
	
			// Malformed message, skip
			if (msg->FindInt32("opcode", &opcode) != B_OK) {
				return;
			};

			ContactHandle handle;
			ContactHandle from;
			ContactHandle to;
			const char *name;
	
			switch (opcode) {
				case B_ENTRY_CREATED: {
					// .. add to contact list
					msg->FindString("name", &name );
					handle.entry.set_name(name);
					msg->FindInt64("directory", &handle.entry.directory);
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);
					
					ContactAdded(handle);
				} break;
				case B_ENTRY_MOVED: {
					// .. contact moved
					msg->FindString("name", &name );
					from.entry.set_name(name);
					to.entry.set_name(name);
					msg->FindInt64("from directory", &from.entry.directory);
					msg->FindInt64("to directory", &to.entry.directory);
					msg->FindInt32("device", &from.entry.device);
					msg->FindInt32("device", &to.entry.device);
					msg->FindInt64("node", &from.node);
					msg->FindInt64("node", &to.node);
					
					ContactMoved(from, to);
				} break;
				case B_ATTR_CHANGED: {
					// .. add to contact list
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);
					
					ContactModified(handle);
				} break;
				case B_ENTRY_REMOVED: {
					// .. remove from contact list
					msg->FindInt64("directory", &handle.entry.directory);
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);
					
					ContactRemoved(handle);
				} break;
			}
		} break;
	
		default: {
			BLooper::MessageReceived(msg);
		} break;
	};
};

//#pragma mark SpecificationFinder<Contact *> Hooks

ContactCachedConnections *ContactManager::FindFirst(ContactSpecification *spec, bool canDelete = true) {
	return fContact->FindFirst(spec, canDelete);
};

GenericListStore<ContactCachedConnections *> ContactManager::FindAll(ContactSpecification *spec, bool canDelete = true) {
	return fContact->FindAll(spec, canDelete);
};

//#pragma mark Public

status_t ContactManager::Init(void) {
	BMessage msg;
	ContactHandle handle;
	BVolumeRoster vroster;
	BVolume vol;
	char volName[B_FILE_NAME_LENGTH];
	
	vroster.Rewind();
		
	// query for all contacts on all drives
	while (vroster.GetNextVolume(&vol) == B_OK) {
		if ((vol.InitCheck() != B_OK) || (vol.KnowsQuery() != true)) {
			continue;
		};
		
		vol.GetName(volName);
		LOG(kAppName, liLow, "ContactManager::Init(): Getting contacts on %s", volName);
		
		BQuery *query = new BQuery();
		query->SetTarget(BMessenger(this));
		query->SetPredicate("IM:connections=*");
		query->SetVolume(&vol);
		query->Fetch();
		
		fQuery->Add(query);
		
		while (query->GetNextRef(&handle.entry) == B_OK) {
			node_ref nref;
			
			BNode node(&handle.entry);
			if (node.GetNodeRef( &nref ) == B_OK) {
				handle.node = nref.node;
				
				ContactAdded(handle);
			};
		};
	};

	return B_OK;
};

//#pragma mark Private

void ContactManager::ContactAdded(ContactHandle handle) {
	// Check if the Contact already exists
	if (fContact->Contains(handle.entry) == true) {
		return;
	};

	// Set up node monitoring
	node_ref nref;
	nref.device = handle.entry.device;
	nref.node = handle.node;
	watch_node(&nref, B_WATCH_ALL, BMessenger(this));

	// Save the Contact
	ContactCachedConnections *contact = new ContactCachedConnections(handle.entry);
	fContact->Add(handle.entry, contact);
	
	// Notify the listener
	fContactListener->ContactAdded(contact);
};

void ContactManager::ContactModified(ContactHandle handle) {
	ContactCachedConnections *contact = fContact->FindFirst(new EntryRefContactSpecification(handle.entry));

	ConnectionStore *prevCons = contact->CachedConnections();
	ConnectionStore *newCons = NULL;

	// Notify the listener
	fContactListener->ContactModified(contact, prevCons, newCons);
	
	// Now that the listener has been notified update its connections
	contact->ReloadConnections();
};

void ContactManager::ContactMoved(ContactHandle from, ContactHandle to) {
//	for ( list< pair<ContactHandle, list<string>* > >::iterator i = fContacts.begin(); i != fContacts.end(); i++ )
//	{
//		if ( (*i).first == from )
//		{
//			(*i).first = to;
//			return;
//		}
//	}
}

void ContactManager::ContactRemoved(ContactHandle handle) {
	// Stop node monitoring
	node_ref nref;
	nref.device = handle.entry.device;
	nref.node = handle.node;	
	watch_node(&nref, B_STOP_WATCHING, BMessenger(this));

	ContactCachedConnections *contact = fContact->FindFirst(new EntryRefContactSpecification(handle.entry));
	ConnectionStore *prevCons = contact->CachedConnections();

	// Notify the listener
	fContactListener->ContactRemoved(contact, prevCons);

	// Remove the Contact
	if (fContact->Contains(handle.entry) == true) {
		fContact->Remove(handle.entry);
	};
}
