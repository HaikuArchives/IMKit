#include "SettingsController.h"

#include <interface/CheckBox.h>
#include <interface/MenuField.h>
#include <interface/MenuItem.h>
#include <interface/TextControl.h>

#include <stdlib.h>

//#pragma mark Hooks

status_t SettingsController::Save(BView *panel, const BMessage *tmplate, BMessage *settings) {
	BMessage cur;

	for (int i = 0; tmplate->FindMessage("setting", i, &cur) == B_OK; i++) {
		const char *name = cur.FindString("name");
		int32 type = -1;
	
		cur.FindInt32("type", &type);
	
		if (dynamic_cast<BTextControl*>(panel->FindView(name))) {
			// Free text
			BTextControl *ctrl = (BTextControl*)panel->FindView(name);
	
			switch (type) {
				case B_STRING_TYPE: {
					settings->AddString(name, ctrl->Text());
				} break;
				case B_INT32_TYPE: {
					settings->AddInt32(name, atoi(ctrl->Text()));
				} break;
				default: {
					return B_ERROR;
				};
			}
		} else if (dynamic_cast<BMenuField *>(panel->FindView(name))) {
			// Provided option
			BMenuField *ctrl = (BMenuField *)panel->FindView(name);
			BMenuItem *item = ctrl->Menu()->FindMarked();
	
			if (!item) {
				return B_ERROR;
			};
	
			switch (type) {
				case B_STRING_TYPE: {
					settings->AddString(name, item->Label());
				} break;
				case  B_INT32_TYPE: {
					settings->AddInt32(name, atoi(item->Label()));
				} break;
				default: {
					return B_ERROR;
				};
			}
		} else if (dynamic_cast<BCheckBox *>(panel->FindView(name))) {
			// Boolean setting
			BCheckBox *box = (BCheckBox *)panel->FindView(name);
	
			if (box->Value() == B_CONTROL_ON) {
				settings->AddBool(name, true);
			} else {
				settings->AddBool(name, false);
			};
		} else if (dynamic_cast<BTextView *>(panel->FindView(name))) {
			BTextView *view = (BTextView *)panel->FindView(name);
			settings->AddString(name, view->Text());
		};
	};
	
	return B_OK;
};

status_t SettingsController::Revert(BView *panel, const BMessage *tmplate) {
	return B_ERROR;
};