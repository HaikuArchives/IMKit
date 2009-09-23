#include "QueryLooper.h"

#include <stdio.h>

QueryLooper::QueryLooper(const char *predicate, vollist vols, const char *name,
	BHandler *notify, BMessage *msg)
	: BLooper(name),
		fMsg(NULL) {
	
	fName = name;
	if ( notify )
		fNotify = BMessenger(notify);
	fPredicate = predicate;
	fVolumes = vols;
	if ( msg )
		fMsg = new BMessage(*msg);
	
	Run();
	
	BMessenger(this).SendMessage(msgInitialFetch);
};

QueryLooper::~QueryLooper(void) {
	if ( fMsg )
		delete fMsg;
	
	querylist::iterator vIt;
	for (vIt = fQueries.begin(); vIt != fQueries.end(); vIt++) {
		(*vIt)->Clear();
		delete (*vIt);
	};
};

void QueryLooper::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case B_QUERY_UPDATE: {
			int32 opcode = 0;
			if (msg->FindInt32("opcode", &opcode) != B_OK) return;
					
			switch (opcode) {
				case B_ENTRY_CREATED: {
					result r;
					const char *name;
				
					msg->FindInt32("device", &r.ref.device); 
					msg->FindInt64("directory", &r.ref.directory); 
					msg->FindString("name", &name); 
					r.ref.set_name(name);

					msg->FindInt32("device", &r.nref.device);
					msg->FindInt64("node", &r.nref.node);
										
					fResults[r.ref] = r;
				} break;
				
				case B_ENTRY_REMOVED: {
					node_ref nref;
					resultmap::iterator rIt;

					msg->FindInt32("device", &nref.device);
					msg->FindInt64("node", &nref.node);

					for (rIt = fResults.begin(); rIt != fResults.end(); rIt++) {
						result r = rIt->second;
						
						if (nref == r.nref) {
							fResults.erase(r.ref);
							break;
						};
					};
				} break;
			};
		
			if ((fNotify.IsValid()) && (fMsg != NULL)) {
				BMessage notify(*fMsg);
				notify.AddString("qlName", fName);

#if B_BEOS_VERSION > B_BEOS_VERSION_5				
				fNotify.SendMessage(notify);
#else
				fNotify.SendMessage(&notify);
#endif
			};
		} break;
		
		case msgInitialFetch: {
			vollist::iterator vIt;
			for (vIt = fVolumes.begin(); vIt != fVolumes.end(); vIt++) {
				BVolume vol = (*vIt);
				BQuery *query = new BQuery();
		
				query->SetPredicate(fPredicate.String());
				query->SetTarget(this);
				query->SetVolume(&vol);
		
				query->Fetch();
		
				entry_ref ref;
				while (query->GetNextRef(&ref) == B_OK) {
					BNode node(&ref);
					result r;

					r.ref = ref;
					node.GetNodeRef(&r.nref);

					fResults[ref] = r;
				};
				
				fQueries.push_back(query);
			};
			
		} break;
		
		default: {
			BLooper::MessageReceived(msg);
		};
	};
};

int32 QueryLooper::CountEntries(void) {
	return fResults.size();
};

entry_ref QueryLooper::EntryAt(int32 index) {
	entry_ref ref;
	if ((index >= 0) && (index < CountEntries())) {
		resultmap::iterator rIt = fResults.begin();
		
		for (int32 i = 0; i < index; index++) rIt++;
		
		ref = rIt->first;
	}
	
	return ref;
};
