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

#include "PServerOverview.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

PServerOverview::PServerOverview(BRect bounds)
	: BView(bounds, "settings", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	BRect frame(0, 0, 1, 1);
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BStringView* serversLabel = new BStringView(frame, NULL, _T("Servers"));
	serversLabel->SetAlignment(B_ALIGN_LEFT);
	serversLabel->SetFont(be_bold_font);

	BBox* divider1 = new BBox(frame, B_EMPTY_STRING, B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_FRAME_EVENTS, B_FANCY_BORDER);

#ifdef __HAIKU__
	serversLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	divider1->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

	// Build the layout
	SetLayout(new BGroupLayout(B_HORIZONTAL));

	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(serversLabel, 0, 0, 2)
			.Add(divider1, 0, 1, 2)
		)

		.AddGlue()
		.SetInsets(inset, inset, inset, inset)
	);
#else
	AddChild(serversLabel);
	AddChild(divider1);
#endif
}
