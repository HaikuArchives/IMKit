/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PPROTOCOLS_OVERVIEW_H
#define PPROTOCOLS_OVERVIEW_H

#include <interface/View.h>

class BStringView;

class Divider;

class PProtocolsOverview : public BView {
	public:
							PProtocolsOverview(BRect bounds);
	
		// BView Hooks
		virtual void		AttachedToWindow(void);
		virtual void		MessageReceived(BMessage *msg);
	
	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif

		BStringView			*fProtocolsLabel;
		Divider				*fProtocolsDivider;

};

#endif // PPROTOCOLS_OVERVIEW_H
