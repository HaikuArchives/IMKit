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

class PSettingsOverview : public BView {
	public:
							PSettingsOverview(BRect bounds);

		// BWindow Hooks
		virtual void		AttachedToWindow();

	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif

		BStringView			*fServerLabel;
		Divider				*fServerDivider;
		MultiLineStringView	*fServerDesc;
		BButton				*fServerButton;
		
		BStringView			*fProtocolsLabel;
		Divider				*fProtocolDivider;
		MultiLineStringView	*fProtocolsDesc;
		BButton				*fProtocolsButton;
			

		BStringView			*fClientsLabel;
		Divider				*fClientsDivider;
		MultiLineStringView	*fClientsDesc;		
		BButton				*fClientsButton;
};

const int32 kMsgEditServer = 'Mesr';
const int32 kMsgEditProtocols = 'Mpro';
const int32 kMsgEditClients = 'Mcli';

#endif // PSETTINGS_OVERVIEW_H
