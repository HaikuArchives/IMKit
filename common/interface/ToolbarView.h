/*
 * Copyright 2009, Maxime Simon. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TOOLBAR_VIEW_H
#define _TOOLBAR_VIEW_H

#include <GroupView.h>

class ToolbarView : public BGroupView {
public:
					ToolbarView();
	virtual			~ToolbarView();

			void	Draw(BRect updateRect);
};

#endif	// _TOOLBAR_VIEW_H
