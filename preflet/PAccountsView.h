#ifndef PACCOUNTS_VIEW_H
#define PACCOUNTS_VIEW_H

#include <interface/View.h>

#include "SettingsController.h"

class BStringView;
class AccountStore;
class Divider;

class PAccountsView : public BView, public SettingsController {
	public:
							PAccountsView(BRect bounds, BPath* protoPath);
							~PAccountsView(void);

		// BView Hooks
		virtual void 		AttachedToWindow(void);
		virtual void		MessageReceived(BMessage *msg);

		// SettingsController Hooks
		virtual status_t	Save(BView *view, const BMessage *tmplate, BMessage *settings);
		virtual status_t	Revert(BView *view, const BMessage *tmplate);

	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif
		void				LoadSettings(bool reload);
		
		BPath				fProtoPath;
		AccountStore		*fSettings;
		
		BStringView		*fHeadingLabel;
		Divider			*fHeadingDivider;
		BOutlineListView	*fProtocolListView;
		BButton				*fAddButton;
		BButton				*fEditButton;
		BButton				*fDelButton;
};

#endif // PACCOUNTS_VIEW_H
