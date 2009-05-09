/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PSERVER_OVERVIEW_H
#define PSERVER_OVERVIEW_H

#include <interface/StringView.h>

#include "ViewFactory.h"

class Divider;

class PServerOverview : public AbstractView {
	public:
						PServerOverview(BRect bounds);

		// BView Hooks
		virtual void			AttachedToWindow(void);
		virtual void			MessageReceived(BMessage *msg);
		
	private:
#ifndef __HAIKU__
		void					LayoutGUI(void);
#endif

		BStringView				*fServerLabel;
		Divider					*fServerDivider;
};

#endif // PSERVER_OVERVIEW_H
