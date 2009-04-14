#include "ContactManager.h"

#include <storage/Directory.h>
#include <storage/Entry.h>
#include <storage/FindDirectory.h>
#include <storage/NodeMonitor.h>
#include <storage/Query.h>
#include <storage/VolumeRoster.h>
#include <support/Autolock.h>
#include <support/Locker.h>

#include <libim/Constants.h>
#include <libim/Contact.h>
#include <libim/Helpers.h>

#include "ContactCachedConnections.h"
#include "ContactListener.h"
#include "common/SpecificationFinder.h"

//#pragma mark Definitions

//#pragma mark Constants

const char *kContactMIMEType = "application/x-person";
extern char *kAppName;

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

		virtual bool FindFirstNoLock(ContactSpecification *specification, ContactCachedConnections **firstMatch, bool deleteSpec = true) {
			bool result = false;
			GenericMapStore<entry_ref, ContactCachedConnections *>::Iterator it;

			for (it = Start(); it != End(); it++) {
				ContactCachedConnections *current = it->second;
				
				if (specification->IsSatisfiedBy(current) == true) {
					*firstMatch = current;
					result = true;
					break;
				};
			};

			if (deleteSpec == true) {
				delete specification;
			};

			return result;
		};
		
		virtual bool FindFirst(ContactSpecification *specification, ContactCachedConnections **firstMatch, bool deleteSpec = true) {
			BAutolock lock(fLock);
			bool result = false;
			
			if (lock.IsLocked() == true) {
				result = FindFirstNoLock(specification, firstMatch, deleteSpec);
			};
			
			
			return result;
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
	
		BLocker *GetLock(void) const {
			return fLock;
		};
	private:
		BLocker	*fLock;
};

//#pragma mark Constructor

ContactManager::ContactManager(ContactListener *listener)
	: BLooper("ContactManager", B_LOW_PRIORITY),
	fContactListener(listener),
	fContact(new ContactStore()), 
	fQuery(new QueryStore()) {
	
	Run();
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
					// Add to contact list
					msg->FindString("name", &name );
					handle.entry.set_name(name);
					msg->FindInt64("directory", &handle.entry.directory);
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);
					
					if (fContact->Contains(handle.entry) == false) {
						ContactAdded(handle);
					} else {
						LOG(kAppName, liDebug, "Got a B_ENTRY_CREATED for a Contact we already know about - possibly due to live query notification catching up with a call to CreateContact");
					};
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
			const char *name = NULL;
	
			switch (opcode) {
				case B_ENTRY_CREATED: {
					// Add to contact list
					msg->FindString("name", &name );
					handle.entry.set_name(name);
					msg->FindInt64("directory", &handle.entry.directory);
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);

					if (fContact->Contains(handle.entry) == false) {					
						ContactAdded(handle);
					} else {
						LOG(kAppName, liDebug, "Got a B_ENTRY_CREATED for a Contact we already know about - possibly due to live query notification catching up with a call to CreateContact");
					};
				} break;
				case B_ENTRY_MOVED: {
					// Contact moved
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
					// Add to contact list
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);
						
					ContactCachedConnections *cached = NULL;
					if (fContact->FindFirst(new ContactHandleSpecification(handle), &cached) == true) {
						ContactModified(cached->Handle());
					} else {
						LOG(kAppName, liDebug, "Got a B_ATTR_CHANGED request for a Contact we cannot match");
					};
				} break;
				
				case B_ENTRY_REMOVED: {
					// Remove from contact list
					msg->FindInt64("directory", &handle.entry.directory);
					msg->FindInt32("device", &handle.entry.device);
					msg->FindInt64("node", &handle.node);

					ContactCachedConnections *cached = NULL;
					if (fContact->FindFirst(new ContactHandleSpecification(handle), &cached) == true) {
						ContactRemoved(handle);
					} else {
						LOG(kAppName, liDebug, "Got a B_ENTRY_REMOVED request for a Contact we cannot match");
					};
				} break;
			}
		} break;
	
		default: {
			BLooper::MessageReceived(msg);
		} break;
	};
};

//#pragma mark SpecificationFinder<Contact *> Hooks

bool ContactManager::FindFirst(ContactSpecification *spec, ContactCachedConnections **firstMatch, bool canDelete = true) {
	return fContact->FindFirst(spec, firstMatch, canDelete);
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
		
	// Query for all contacts on all drives
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
			if (node.GetNodeRef(&nref) == B_OK) {
				handle.node = nref.node;		
				ContactAdded(handle);
			};
		};
	};

	return B_OK;
};

/**
	Create a new People file with a unique name on the form "Unknown contact X"
	and add the specified connection to it
	
	@param connection The Connection object of the new contact
	@param namebase The name to save the file as (will be appended with an integer if it already exists)
*/

ContactCachedConnections *ContactManager::CreateContact(Connection connection, const char *namebase) {
	ContactCachedConnections *result = NULL;

	BAutolock lock(fContact->GetLock());
	if (lock.IsLocked() == false) {
		LOG(kAppName, liHigh,
			"ContactManager::CreateContact: Unable to acquire storage lock");
		return result;
	};

	if (fContact->FindFirstNoLock(new ConnectionContactSpecification(connection), &result) == true) {
		LOG(kAppName, liHigh,
			"CreateContact for a connection that already exists - returning existing item");
		LOG(kAppName, liHigh, "    %s", result->EntryRef().name);
		return result;
	};

	BPath path;

	if (find_directory(B_USER_DIRECTORY, &path, true, NULL) != B_OK) {
		LOG(kAppName, liHigh, "Unable to find B_USER_DIRECTORY");
		return result;
	};

	path.Append("people");

	// make sure that the target directory exists before we try to create new files
	create_directory(path.Path(), 0777);

	BDirectory dir(path.Path());
	BFile file;
	BEntry entry;
	char filename[512];

	// Make sure we have a decent namebase
	if ((namebase == NULL) || (strlen(namebase) == 0)) {
		namebase = "Unknown contact";
	};

	strcpy(filename, namebase);

	// Attempt to create the contact
	dir.CreateFile(filename, &file, true);

	for (int i = 1; file.InitCheck() != B_OK; i++) {
		sprintf(filename, "%s %d", namebase, i);
		
		dir.CreateFile(filename, &file, true);
	};

	if (dir.FindEntry(filename,&entry) != B_OK) {
		LOG(kAppName, liHigh,
			"Error: While creating a new contact, dir.FindEntry() failed. filename was [%s]",
			filename);
		return result;
	};

	// File created - set the type
	if (file.WriteAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0, kContactMIMEType, strlen(kContactMIMEType)) != strlen(kContactMIMEType)) {
		// Error writing type
		entry.Remove();
		
		LOG(kAppName, liHigh, "ERROR: Unable to write MIME type for newly created contact");
		return result;
	};

	// File created. set type and add connection
	result = new ContactCachedConnections(entry);
	if (result->AddConnection(connection.String()) != B_OK) {
		delete result;
		result = NULL;
		
		return result;
	};

	if (result->SetStatus(OFFLINE_TEXT) != B_OK) {
		delete result;

		result = NULL;
		return result;
	};

	// Ensure the cached contacts are up to date
	result->ReloadConnections();

	fContact->Add(result->EntryRef(), result);

	return result;
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
	contact->ReloadConnections();
	fContact->Add(handle.entry, contact);
	
	// Notify the listener
	fContactListener->ContactAdded(contact);
};

void ContactManager::ContactModified(ContactHandle handle) {
	ContactCachedConnections *contact = NULL;

	if (fContact->FindFirst(new ContactHandleSpecification(handle), &contact) == true) {
		ConnectionStore *temp = contact->CachedConnections();
		// Copy the previous connections
		ConnectionStore prevCons;
		for (ConnectionStore::Iterator it = temp->Start(); it != temp->End(); it++) {
			Connection con = (*it);
			prevCons.Add(con);
		};

		temp = NULL;
		contact->ReloadConnections();
		ConnectionStore *newCons = contact->CachedConnections();

		// Notify the listener
		fContactListener->ContactModified(contact, &prevCons, newCons);
	} else {
		LOG(kAppName, liHigh, "ContactManager::ContactModified: %s is an unknown contact", handle.entry.name);
	};
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
};

void ContactManager::ContactRemoved(ContactHandle handle) {
	// Stop node monitoring
	node_ref nref;
	nref.device = handle.entry.device;
	nref.node = handle.node;	
	watch_node(&nref, B_STOP_WATCHING, BMessenger(this));

	ContactCachedConnections *contact = NULL;
	if (fContact->FindFirst(new ContactHandleSpecification(handle), &contact) == true) {
		ConnectionStore *prevCons = contact->CachedConnections();
	
		// Notify the listener
		fContactListener->ContactRemoved(contact, prevCons);
	
		// Remove the Contact
		if (fContact->Contains(handle.entry) == true) {
			fContact->Remove(handle.entry);
		};
	} else {
		LOG(kAppName, liHigh, "ContactManager::ContactRemoved: %s is an unknown contact", handle.entry.name);
	};
};
