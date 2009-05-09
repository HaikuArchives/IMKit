/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#ifdef __HAIKU__
#	include <interface/CardLayout.h>
#endif

#include "PCardView.h"

//#pragma mark Constructors

PCardView::PCardView(BRect bounds)
	: AbstractView(bounds, "box", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	fCurrentView(NULL)
{
#ifdef __HAIKU__
	SetLayout(new BCardLayout());
#endif
}

void PCardView::Add(BView *view) {
#ifdef __HAIKU__
	BCardLayout* layout
		= dynamic_cast<BCardLayout*>(GetLayout());
	layout->AddView(view);
#else
	AddChild(view);
#endif
}

void PCardView::Select(BView *view) {
#ifdef __HAIKU__
	BCardLayout* layout
		= dynamic_cast<BCardLayout*>(GetLayout());
	layout->SetVisibleItem(layout->IndexOfView(view));
#else
	if (fCurrentView != NULL)
		fCurrentView->Hide();
	fCurrentView = view;
	fCurrentView->Show();
#endif
}
