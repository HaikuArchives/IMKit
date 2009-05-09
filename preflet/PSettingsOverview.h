/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PSETTINGS_OVERVIEW_H
#define PSETTINGS_OVERVIEW_H

#include <interface/View.h>

#include "SettingsController.h"
#include "ViewFactory.h"

class BButton;
class BStringView;

class Divider;
class MultiLineStringView;
class MultipleViewHandler;

class PSettingsOverview : public AbstractView, public SettingsController {
	public:
					PSettingsOverview(MultipleViewHandler *handler, BRect bounds);

		// BView Hooks
		virtual void		AttachedToWindow(void);
		virtual void		MessageReceived(BMessage *msg);

		// SettingsController Hooks
		virtual status_t	Init(SettingsHost *host);
		virtual status_t	Save(const BMessage *tmplate, BMessage *settings);
		virtual status_t	Revert(const BMessage *tmplate);		

	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif
		MultipleViewHandler	*fHandler;

		BStringView			*fServerLabel;
		Divider				*fServerDivider;
		MultiLineStringView	*fServerDesc;
		BButton				*fServerButton;
		
		BStringView			*fProtocolsLabel;
		Divider				*fProtocolsDivider;
		MultiLineStringView	*fProtocolsDesc;
		BButton				*fProtocolsButton;
			

		BStringView			*fClientsLabel;
		Divider				*fClientsDivider;
		MultiLineStringView	*fClientsDesc;		
		BButton				*fClientsButton;
};

#endif // PSETTINGS_OVERVIEW_H
