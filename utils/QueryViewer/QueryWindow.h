#ifndef QUERYWINDOW_H
#define QUERYWINDOW_H

#include <Application.h>
#include <Alert.h>
#include <MessageRunner.h>
#include <NodeMonitor.h>
#include <OutlineListView.h>
#include <ScrollView.h>
#include <View.h>
#include <Window.h>

#include <vector>

#include "IconCountItem.h"
#include "QueryColumnListView.h"

#ifdef __HAIKU__
#	include <common/columnlistview/haiku/ColumnListView.h>
#	include <common/columnlistview/haiku/ColumnTypes.h>
#else
#	include <common/columnlistview/zeta/ColumnListView.h>
#	include <common/columnlistview/zeta/ColumnTypes.h>
#endif
#include <common/IMKitUtilities.h>

typedef std::map<BString, QueryColumnListView *> cl_map; // Path / QCLV map
typedef std::map<entry_ref, IconCountItem *> di_map; // Directory / IconTextItem map

class QueryWindow : public BWindow {
	public:
							QueryWindow(BRect rect);
							~QueryWindow(void);
						
			virtual	void	MessageReceived(BMessage *msg);
	
	private:
					enum {
							qwQuerySelected = 'qw01',
							qwQueryUpdated = 'qw02',
							qwQueryInvoked = 'qw03',
					};
			
					void	CreateGroups(BDirectory dir, BListItem *under, BRect rect);
					int32	CountItemEntries(IconCountItem *item);
	
					BView	*fView;
		BOutlineListView	*fQueryList;
			IconCountItem	*fRootItem;
		
					BRect	fListRect;

					di_map	fDirectoryItem;
					cl_map	fQueryViews;
		QueryColumnListView	*fCurrentQView;
};

#endif
