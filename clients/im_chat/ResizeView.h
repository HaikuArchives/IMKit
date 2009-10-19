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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Ted Stodgell <kart@hal-pc.org>
 */
 
#ifndef _RESIZEVIEW_H
#define _RESIZEVIEW_H

#include "main.h"

#include <View.h>
#include <Cursor.h>

// horizontal resize cursor taken from OpenTracker, see www.opentracker.org for license

const unsigned char Mak_Vcursor[] =
{
	16,1,
	8,8,
	0x00,0x00,	//pixel data
	0x00,0x00,
	0x01,0x00,
	0x03,0x80,
	0x07,0xc0,
	0x0F,0xe0,
	0x00,0x00,
	0x7f,0xfc,
	0x7f,0xfc,
	0x00,0x00,
	0x0F,0xe0,
	0x07,0xc0,
	0x03,0x80,
	0x01,0x00,
	0x00,0x00,
	0x00,0x00,
	
	0x00,0x00,	//transparency data
	0x00,0x00,
	0x01,0x00,
	0x03,0x80,
	0x07,0xc0,
	0x0F,0xe0,
	0x00,0x00,
	0x7f,0xfc,
	0x7f,0xfc,
	0x00,0x00,
	0x0F,0xe0,
	0x07,0xc0,
	0x03,0x80,
	0x01,0x00,
	0x00,0x00,
	0x00,0x00,
};

class ResizeView : public BView
{
  public:
    ResizeView (BView *, BRect, const char * = "resizeView", uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, uint32 = 0);
    virtual ~ResizeView (void);
    virtual void MouseDown(BPoint);
    virtual void MouseMoved (BPoint, uint32, const BMessage *);
    virtual void MouseUp (BPoint);
  
  private:
    bool mousePressed;
    BView *attachedView;
    BCursor cursor;
};

#endif // _RESIZEVIEW_H
