/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_VIEW_H
#define _STATUS_VIEW_H

#include <interface/View.h>

#include <libim/AccountInfo.h>

class BPopUpMenu;
class BMenuField;
class BMenuItem;

class StatusView : public BView {
public:
								StatusView(const char* name);

	// BView hooks
	virtual	void				AttachedToWindow();

	// Public methods
			void				GetProtocolStatuses();

private:
			IM::AccountStatus	fStatus;
			BPopUpMenu* 		fStatusMenu;
			BMenuField*			fStatusMenuField;
			BMenuItem*			fOnlineMenuItem;
			BMenuItem*			fAwayMenuItem;
			BMenuItem*			fOfflineMenuItem;
};

#endif	// _STATUS_VIEW_H
