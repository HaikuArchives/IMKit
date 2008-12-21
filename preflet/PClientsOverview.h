/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PCLIENTS_OVERVIEW_H
#define PCLIENTS_OVERVIEW_H

#include <interface/View.h>
#include <support/String.h>

#include <vector>

class Divider;
class MultipleViewHandler;
class MultiLineStringView;

class ClientInfo;

typedef vector<ClientInfo *> clientinfo_t;

class PClientsOverview : public BView {
	public:
							PClientsOverview(MultipleViewHandler *handler, BRect bounds);
	
		// BView Hooks
		virtual void		AttachedToWindow(void);
		virtual void		MessageReceived(BMessage *msg);
	
	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif

		MultipleViewHandler	*fHandler;
		
		BStringView			*fAutoStartLabel;
		Divider				*fAutoStartDivider;
		MultiLineStringView	*fAutoStartDesc;
		
		clientinfo_t		fClientInfo;
};

#endif // PCLIENTS_OVERVIEW_H
