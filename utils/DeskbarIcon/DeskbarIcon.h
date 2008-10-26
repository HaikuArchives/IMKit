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
#include <Resources.h>
#include <String.h>
#include <TextView.h>
#include <View.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include <list>
#include <map>
#include <string>

#include <common/IconMenuItem.h>
#include <common/IMKitUtilities.h>
#include <common/BubbleHelper.h>
#include "AwayMessageWindow.h"
#include "QueryLooper.h"

#include <libim/Manager.h>
#include <libim/Contact.h>
#include <kernel/fs_attr.h>
#include <kernel/fs_info.h>

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
		enum {
			SET_STATUS		= 'set1',
			SET_ONLINE		= 'set2',
			SET_AWAY		= 'set3',
			SET_OFFLINE		= 'set4',

			
			OPEN_SETTINGS	= 'opse',
			RELOAD_SETTINGS = 'upse',
			
			CLOSE_IM_SERVER = 'imqu',
			START_IM_SERVER = 'imst',
			
			SETTINGS_WINDOW_CLOSED = 'swcl',
			
			OPEN_QUERY		= 'opqu',
			LAUNCH_FILE		= 'lafi',			
			OPEN_QUERY_DIR	= 'opqd',
			QUERY_UPDATED = 'qlup'
		};
		enum {
			isOffline = 0,
			isAway = 1,
			isOnline = 2
		};
			
		void				_init();
		void				reloadSettings();
		void				getProtocolStates();
		
		BResources			fResource;
		
		BBitmap				*fCurrIcon;
		BBitmap				*fModeIcon;
		BBitmap				*fOnlineIcon;
		BBitmap				*fOfflineIcon;
		BBitmap				*fAwayIcon;
		BBitmap 			*fFlashIcon;
		
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
		map<string, string>	fStatuses;
		bool				fDirtyStatus;		// Are our statuses out of date?
		map<string, string>	fFriendlyNames;
		
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
#define DESKBAR_ICON_SIG "application/x-vnd.beclan.im_kit-DeskbarIcon"


#endif
