/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <LayoutUtils.h>

#include "ContactListView.h"
#include "PeopleColumnListView.h"
#include "StatusView.h"

ContactListView::ContactListView(const char* name)
	: BView(name, B_WILL_DRAW)
{
	// Views
	fListView = new PeopleColumnListView("contact_list");
	fStatusView = new StatusView("status");

	// Populate list view
	fListView->Populate();

	// Layout
	float inset = (float)ceilf(be_plain_font->Size() * 0.7f);
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(fListView)
		.Add(fStatusView)
	);
}


BSize
ContactListView::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), BSize(600, 350));
}
