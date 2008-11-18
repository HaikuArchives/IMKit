/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PVIEW_H
#define PVIEW_H

#include <interface/View.h>
#include <interface/Button.h>

#include <map>

class BOutlineListView;
class IconTextItem;

typedef map<BString, BView*> view_map;
typedef pair<BMessage, BMessage> addons_pair;
typedef map<BString, addons_pair> addons_map;

class PView : public BView {
	public:
						PView(BRect bounds);

		virtual void	AttachedToWindow();
		virtual void	MessageReceived(BMessage* msg);

	private:
			void		LoadProtocols();
			void		LoadClients();
			float		BuildGUI(BMessage viewTemplate, BMessage settings, BView* view);

	private:
		BOutlineListView*	fListView;
		IconTextItem*		fServersItem;
		IconTextItem*		fProtocolsItem;
		IconTextItem*		fClientsItem;

		BView*				fMainView;
		BButton*			fRevert;
		BButton*			fSave;

		view_map			fViews;
		addons_map			fAddOns;

		BView*				fCurrentView;
		int32				fCurrentIndex;

		float				fFontHeight;
};

#endif // PVIEW_H
