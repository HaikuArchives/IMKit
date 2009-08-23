/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is Vision.
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 * 
 * Contributor(s): Rene Gollent
 *                 Alan Ellis <alan@cgsoftware.org>
 */
 
//------------------------------------------------------------------------------
// IconMenu.cpp
//------------------------------------------------------------------------------
// A menu item implementation that displays an icon as its label.
//
// IconMenu implementation Copyright (C) 1998 Tyler Riti <fizzboy@mail.utexas.edu>
// Based on code Copyright (C) 1997 Jens Kilian
// This code is free to use in any way so long as the credits above remain intact.
// This code carries no warranties or guarantees of any kind. Use at your own risk.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// I N C L U D E S
//------------------------------------------------------------------------------

#include "IconMenuItem.h"

const float kIconTextPadding = 5.0;
const float kEdgePadding = 2.0;
const float kTickSpacing = 16.0;

//------------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//------------------------------------------------------------------------------

IconMenuItem::IconMenuItem(BBitmap* icon, const char *label, const char *extra,
	BMessage *msg, bool ownIcon) :
	BMenuItem(label, msg),
	fIcon(icon),
	fExtra(extra),
	fLabel(label),
	fOwnIcon(ownIcon),
	fCreatedIcon(false) {

	if (fIcon == NULL) {
		fCreatedIcon = true;
		fIcon = new BBitmap(BRect(0, 0, 15, 15), B_COLOR_8_BIT, true);
		char *bits = (char *)fIcon->Bits();
		int32 length = fIcon->BitsLength();
		for (int32 i = 0; i < length; i++) bits[i] = B_TRANSPARENT_MAGIC_CMAP8; 
	};

	be_plain_font->GetHeight(&fFontHeight);
	fFontHeightTotal = fFontHeight.ascent + fFontHeight.descent + fFontHeight.leading;

	_CalculateOffsets();
};

IconMenuItem::~IconMenuItem() {
	if ((fOwnIcon == true) || (fCreatedIcon == true)) delete fIcon;
};

void IconMenuItem::_CalculateOffsets(void) {
	fBounds = fIcon->Bounds();
	
	if (fFontHeightTotal > fBounds.Height()) fBounds.bottom = fFontHeightTotal;
	fBounds.bottom += kEdgePadding;

	float width = be_plain_font->StringWidth(fLabel.String());

	fBounds.right += width + kIconTextPadding + kTickSpacing + kEdgePadding;
};

void IconMenuItem::GetContentSize(float* width, float* height) {
	*width = fBounds.Width();
	*height = fBounds.Height();
}

void IconMenuItem::DrawContent() {
	BRect		b = Frame();
	BMenu		*parent = Menu();
	BPoint		loc = parent->PenLocation();
	
	parent->PushState();
	
	parent->SetDrawingMode(B_OP_ALPHA);
	
	b.OffsetBy(0, kEdgePadding);
	b.left = loc.x;

	BRect iconPos = b;
	iconPos.right = b.left + fIcon->Bounds().Width();
	iconPos.bottom = iconPos.top + fIcon->Bounds().Height();
	parent->DrawBitmap(fIcon, iconPos);
		
	parent->MovePenTo(loc.x + fIcon->Bounds().Width() + kEdgePadding,
		loc.y + fFontHeight.ascent + kEdgePadding);
	parent->SetDrawingMode( B_OP_OVER );
	parent->DrawString(fLabel.String());

	parent->PopState();
};
