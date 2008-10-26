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
// IconMenu.h
//------------------------------------------------------------------------------
// A menu item implementation that displays an icon as its label.
//
// IconMenu implementation Copyright (C) 1998 Tyler Riti <fizzboy@mail.utexas.edu>
// Based on code Copyright (C) 1997 Jens Kilian
// This code is free to use in any way so long as the credits above remain intact.
// This code carries no warranties or guarantees of any kind. Use at your own risk.
//------------------------------------------------------------------------------

#ifndef ICON_MENU_ITEM_H
#define ICON_MENU_ITEM_H

//------------------------------------------------------------------------------
// I N C L U D E S
//------------------------------------------------------------------------------

#include <interface/MenuItem.h>
#include <interface/Rect.h>
#include <interface/Bitmap.h>
#include <String.h>

//------------------------------------------------------------------------------
// D E C L A R A T I O N S
//------------------------------------------------------------------------------

class BBitmap;
class BRect;

class IconMenuItem : public BMenuItem {
	public:
					IconMenuItem(BBitmap* icon, const char *label,
						const char *extra, BMessage *msg = NULL, bool ownIcon = true);
	    virtual 	~IconMenuItem();

		const char	*Extra(void) { return fExtra.String(); };

	protected:
	    virtual void GetContentSize(float* width, float* height);
	    virtual void DrawContent();

	private:
		void		_CalculateOffsets(void);
		
		BBitmap		*fIcon;
		BString		fExtra;
		BString 	fLabel;
		BRect		fBounds;

		float		fFontHeightTotal;
		font_height	fFontHeight;
		
		bool		fOwnIcon;
		bool		fCreatedIcon;
};

#endif
