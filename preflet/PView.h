/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PVIEW_H
#define PVIEW_H

#include <interface/View.h>
#include <interface/Button.h>

#include <libim/Manager.h>

#include <map>

#include "MultipleViewHandler.h"
#include "SettingsHost.h"

class BOutlineListView;
class IconTextItem;
class SettingsController;

typedef map<BString, BView*> view_map;
typedef pair<BMessage, BMessage> addons_pair;
typedef map<BString, addons_pair> addons_map;

class PView : public BView, public MultipleViewHandler, public SettingsHost {
	public:
								PView(BRect bounds);

		// BView Hooks
		virtual void			AttachedToWindow(void);
		virtual void			MessageReceived(BMessage *msg);

		// MultipleViewHandler Hooks
		virtual void			ShowServerOverview(void);
		virtual void			ShowProtocolsOverview(void);
		virtual void			ShowClientsOverview(void);

		// SettingsHost Hooks
		virtual void			ControllerModified(SettingsController *controller);

	private:
		void					LoadProtocols(void);
		void					LoadClients(void);

		void 					SaveSettings(void);
		void					RevertSettings(void);

	private:
		BOutlineListView		*fListView;
		IconTextItem			*fServerItem;
		IconTextItem			*fProtocolsItem;
		IconTextItem			*fClientsItem;

		BView					*fMainView;
		BButton					*fRevert;
		BButton					*fSave;

		view_map				fViews;
		addons_map				fAddOns;

		BView*					fCurrentView;
		int32					fCurrentIndex;

		float					fFontHeight;

		IM::Manager				*fManager;
};

#endif // PVIEW_H
	