/*
 * Copyright 2009, Maxime Simon. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <ControlLook.h>

#include "ToolbarView.h"


ToolbarView::ToolbarView()
	: BGroupView(B_HORIZONTAL, 5)
{
	SetFlags(Flags() | B_WILL_DRAW);
	SetViewColor(ui_color(B_MENU_BACKGROUND_COLOR));
}


ToolbarView::~ToolbarView()
{
}


void
ToolbarView::Draw(BRect updateRect)
{
	BRect rect = Bounds();

	be_control_look->DrawMenuBarBackground(this, rect, updateRect,
		ui_color(B_MENU_BACKGROUND_COLOR));

	be_control_look->DrawBorder(this, rect, updateRect,
		ui_color(B_MENU_BACKGROUND_COLOR), B_FANCY_BORDER, 0,
		BControlLook::B_BOTTOM_BORDER);
}
