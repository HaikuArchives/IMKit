#ifndef PWINDOW_H
#define PWINDOW_H

#include "main.h"

#include <common/IconTextItem.h>

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Entry.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include "ObjectList.h"
#include <OutlineListView.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>
#include <View.h>
#include <Window.h>

#include <libim/Constants.h>
#include <libim/Manager.h>

#include <common/IMKitUtilities.h>
#include <common/BubbleHelper.h>

#include <map>

class BWindow;
class BButton;

typedef map<BString, BView *> view_map;

class PWindow : public BWindow {
	enum {
		LISTCHANGED,
		SAVE,
		REVERT
	};

	public:
		
								PWindow(void);

		virtual bool			QuitRequested(void);
		virtual void			DispatchMessage(BMessage *msg, BHandler *target);
		virtual void			MessageReceived(BMessage *msg);

				
	private:
		float					BuildGUI(BMessage viewTemplate, BMessage settings,
									BView *view);
	
		BView					*fView;
		BButton					*fSave;
		BButton					*fRevert;
		BOutlineListView		*fListView;
		BBox					*fBox;

		view_map				fPrefViews;		
		BView					*fCurrentView;
		int32					fCurrentIndex;

		IM::Manager				*fManager;
		
//		Stored a pair<Settings, Template>
		map<BString, pair<BMessage, BMessage> >
								fAddOns;

		float					fFontHeight;
};

#endif
