/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PSETTINGS_OVERVIEW_H
#define PSETTINGS_OVERVIEW_H

#include <interface/View.h>

class BButton;

class PSettingsOverview : public BView {
	public:
							PSettingsOverview(BRect bounds);

		virtual void		AttachedToWindow();

	private:
		BButton				*fServerButton;
		BButton				*fClientsButton;
		BButton				*fProtocolsButton;
};

const int32 kMsgEditServer = 'Mesr';
const int32 kMsgEditProtocols = 'Mpro';
const int32 kMsgEditClients = 'Mcli';

#endif // PSETTINGS_OVERVIEW_H
