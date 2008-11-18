/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <interface/StringView.h>
#include <interface/Box.h>
#include <interface/Button.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/GridLayoutBuilder.h>
#	include <interface/SpaceLayoutItem.h>
#endif

#include "PClientsOverview.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

PClientsOverview::PClientsOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect frame(0, 0, 1, 1);
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BStringView* clientsLabel = new BStringView(frame, NULL, _T("Clients"));
	clientsLabel->SetAlignment(B_ALIGN_LEFT);
	clientsLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BBox* divider1 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);
	divider1->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

#ifdef __HAIKU__
	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(clientsLabel, 0, 0, 2)
			.Add(divider1, 0, 1, 2)
		)

		.AddGlue()
		.SetInsets(inset, inset, inset, inset)
	);
#else
	AddChild(clientsLabel);
#endif
}
