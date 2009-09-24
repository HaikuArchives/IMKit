#ifndef QUERYLOOPER_H
#define QUERYLOOPER_H

#include <Entry.h>
#include <Looper.h>
#include <MessageFilter.h>
#include <NodeMonitor.h>
#include <Menu.h>
#include <Query.h>
#include <String.h>
#include <Volume.h>

#include <common/interface/IconMenuItem.h>
#include <common/IMKitUtilities.h>

#include <map>
#include <vector>

typedef struct {
	entry_ref ref;
	node_ref nref;
} result;

typedef std::map<entry_ref, result> resultmap;

typedef std::vector<BVolume> vollist;
typedef std::vector<BQuery *> querylist;

class QueryLooper : public BLooper {
	public:
						QueryLooper(const char *predicate, vollist vols,
							const char *name = NULL, BHandler *notify = NULL,
							BMessage *msg = NULL);
		virtual			~QueryLooper(void);

		virtual	void	MessageReceived(BMessage *msg);

				int32	CountEntries(void);
			entry_ref	EntryAt(int32 index);
	
	private:
			enum {
				msgInitialFetch = 'ql01'
			};
	
			BMessage	*fMsg;	
			//BHandler	*fNotify;	
			BMessenger	fNotify;
			
			resultmap	fResults;
			
			BString		fName;

			vollist		fVolumes;
			querylist	fQueries;
			BString		fPredicate;
};

#endif
