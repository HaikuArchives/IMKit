/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PACCOUNTS_VIEW_H
#define PACCOUNTS_VIEW_H

#include <interface/View.h>

class BButton;
class BOultineListView;
class BPath;
class BWindow;


class PAccountsView : public BView
{
	public:
							PAccountsView(BRect bounds, BPath* protoPath);

		// BView Hooks
		virtual void		AttachedToWindow(void);
		virtual void		MessageReceived(BMessage *msg);

	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif
		
		BPath				*fProtoPath;
		BOutlineListView	*fProtocolListView;
		BButton				*fAddButton;
		BButton				*fEditButton;
		BButton				*fDelButton;
};

#endif // PACCOUNTS_VIEW_H
