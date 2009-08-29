/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_VIEW_H
#define _STATUS_VIEW_H

#include <interface/View.h>

class BPopUpMenu;
class BMenuField;

class StatusView : public BView {
public:
						StatusView(const char* name);

	virtual	void		AttachedToWindow();

private:
			BPopUpMenu* fStatusMenu;
			BMenuField* fStatusMenuField;
};

#endif	// _STATUS_VIEW_H
