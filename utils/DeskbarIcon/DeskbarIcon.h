#ifndef DESKBAR_ICON_H
#define DESKBAR_ICON_H

#include <Alert.h>
#include <Bitmap.h>
#include <Directory.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <NodeMonitor.h>
#include <PopUpMenu.h>
#include <Query.h>
#include <String.h>
#include <TextView.h>
#include <View.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include <list>
#include <map>
#include <string>

#include "AwayMessageWindow.h"
#include "QueryLooper.h"
#include "DeskbarIconResources.h"

#include <libim/Manager.h>
#include <libim/Contact.h>
#include <kernel/fs_attr.h>
#include <kernel/fs_info.h>

class AccountStore;
class AccountInfo;
class BubbleHelper;

typedef struct {
	entry_ref ref;
	node_ref nref;
	BBitmap *icon;
	QueryLooper *query;
} queryinfo;

typedef map<entry_ref, queryinfo> querymap;

class _EXPORT IM_DeskbarIcon : public BView
{
	public:
		IM_DeskbarIcon();
		IM_DeskbarIcon( BMessage * );
		virtual ~IM_DeskbarIcon();
		
		virtual status_t Archive( BMessage * archive, bool deep = true ) const;
		static BArchivable * Instantiate( BMessage * archive );
		
		virtual void Draw( BRect );
		
		virtual void MessageReceived( BMessage * );
		
		virtual void MouseDown( BPoint );
		virtual void MouseMoved(BPoint point, uint32 transit, const BMessage *msg);
		
		virtual void AttachedToWindow();
		virtual void DetachedFromWindow();
		
		virtual void AboutRequested();
		
	private:
		void				_init();
		void				reloadSettings();
		void				getProtocolStates();
		
		BBitmap				*fCurrIcon;
		BBitmap				*fModeIcon;
		BBitmap				*fOnlineIcon;
		BBitmap				*fOfflineIcon;
		BBitmap				*fAwayIcon;
		BBitmap 			*fGenericIcon;
		
		int					fStatus;
		BubbleHelper		*fTip;
				
//		Flashing stuff
		int					fFlashCount, fBlink;
		list<BMessenger>	fMsgrs;
		BMessageRunner *	fMsgRunner;
		
//		Settings
		bool				fShouldBlink;
		const char			*fPeopleApp;

		BString				fTipText;
		BPopUpMenu			*fMenu;	

//		Status menu
		bool				fDirtyStatusMenu;	// Does the menu need rebuilding?
		BMenu				*fStatusMenu;
		
		AccountStore		*fAccount;
		bool				fDirtyStatus;		// Are our statuses out of date?
		
//		Query Menu stuff
		querymap			fQueries;
		BMenu				*fQueryMenu;
		void				RemoveQueryRef(BMessage *msg);
		void				AddQueryRef(BMessage *msg);
		void				BuildQueryMenu(void);
		status_t			ExtractVolumes(BNode *node, vollist *volumes);
		
		entry_ref			fMiddleClickRef;
};

extern "C" _EXPORT BView * instantiate_deskbar_item();

#define DESKBAR_ICON_NAME "IM_DeskbarIcon"

#endif
