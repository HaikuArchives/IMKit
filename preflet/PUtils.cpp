/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <stdlib.h>

#include <app/Message.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif
#include <interface/Window.h>
#include <interface/ScrollView.h>
#include <interface/Screen.h>
#include <interface/Box.h>
#include <interface/CheckBox.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <interface/MenuField.h>
#include <interface/PopUpMenu.h>
#include <interface/TextControl.h>
#include <interface/TextView.h>
#include <interface/StringView.h>

#include <common/BubbleHelper.h>

#include <libim/Helpers.h>

#include "common/Divider.h"
#include "PUtils.h"
#include "ViewFactory.h"

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

const float kControlOffset = 5.0;
const float kEdgeOffset = 5.0;
const float kDividerWidth = 100;

//BubbleHelper gHelper;

void CenterWindowOnScreen(BWindow* window) {
	BRect screenFrame = BScreen().Frame();
	BPoint pt;

	pt.x = screenFrame.Width()/2 - window->Bounds().Width()/2;
	pt.y = screenFrame.Height()/2 - window->Bounds().Height()/2;

	if (screenFrame.Contains(pt))
		window->MoveTo(pt);
}

status_t SaveSettings(BView *panel, BMessage tmplate, BMessage *settings) {
	BMessage cur;
		
	for (int i = 0; tmplate.FindMessage("setting", i, &cur) == B_OK; i++) {
		const char *name = cur.FindString("name");
		int32 type = -1;
		
		cur.FindInt32("type", &type);
		
		if (dynamic_cast<BTextControl*>(panel->FindView(name))) { 
			// Free text
			BTextControl * ctrl = (BTextControl*)panel->FindView(name);
		
			switch (type) {
				case B_STRING_TYPE: {
					settings->AddString(name, ctrl->Text() );
				} break;
				case B_INT32_TYPE: {
					settings->AddInt32(name, atoi(ctrl->Text()) );
				} break;
				default: {
					return B_ERROR;
				};
			};
		} else if (dynamic_cast<BMenuField*>(panel->FindView(name))) {
			// Provided option
			BMenuField * ctrl = (BMenuField*)panel->FindView(name);
			BMenuItem * item = ctrl->Menu()->FindMarked();
			
			if (!item) return B_ERROR;
			
			switch (type) {
				case B_STRING_TYPE: {
					settings->AddString(name, item->Label() );
				} break;
				case  B_INT32_TYPE: {
					settings->AddInt32(name, atoi(item->Label()) );
				} break;
				default: {
					return B_ERROR;
				};
			}
		} else
		if (dynamic_cast<BCheckBox*>(panel->FindView(name))) {
			// Boolean setting
			BCheckBox * box = (BCheckBox*)panel->FindView(name);
			
			if ( box->Value() == B_CONTROL_ON ) {
				settings->AddBool(name,true);
			} else {
				settings->AddBool(name,false);
			}
		} else if (dynamic_cast<BTextView *>(panel->FindView(name))) {
			BTextView *view = (BTextView *)panel->FindView(name);
			settings->AddString(name, view->Text());
		};
	};
	
	return B_OK;
};
