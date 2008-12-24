/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PSETTINGS_OVERVIEW_H
#define PSETTINGS_OVERVIEW_H

#include <interface/View.h>

class BButton;
class BStringView;

class Divider;
class MultiLineStringView;
class MultipleViewHandler;

class PSettingsOverview : public BView {
	public:
							PSettingsOverview(MultipleViewHandler *handler, BRect bounds);

		// BView Hooks
		virtual void		AttachedToWindow();
		virtual void		MessageReceived(BMessage *msg);

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
