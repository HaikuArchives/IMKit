/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *		Michael Davidson <slaad@bong.com.au>
 */

#include <interface/CheckBox.h>
#include <interface/Menu.h>
#include <interface/MenuItem.h>
#include <interface/MenuField.h>
#include <interface/PopUpMenu.h>
#include <interface/ScrollView.h>
#include <interface/StringView.h>
#include <interface/TextControl.h>
#include <interface/TextView.h>

#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif

#ifdef ZETA
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

#include <stdio.h>
#include <string.h>

#include <libim/Helpers.h>

#include "PClientView.h"
#include "PUtils.h"
#include "SettingsHost.h"
#include "ViewFactory.h"

#include "common/BubbleHelper.h"
#include "common/Divider.h"
#include "common/NotifyingTextView.h"

//#pragma mark Global

BubbleHelper gHelper;

//#pragma mark Constants

const int32 kMsgControlChanged = 'mcch';

const float kControlOffset = 5.0;
const float kEdgeOffset = 5.0;
const float kDividerWidth = 100;

//#pragma mark Constuctor

PClientView::PClientView(BRect frame, const char *name, const char *title, BMessage tmplate, BMessage settings)
	: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fTitle(title),
	fTemplate(tmplate),
	fSettings(settings),
	fShowHeading(true),
	fHost(NULL) {
	
	fShowHeading = (title != NULL);
};

//#pragma mark BView Hooks

void PClientView::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif

	fRequiredHeight = BuildGUI();

	BView *parent = this;
#ifdef __HAIKU__
	// On Haiku we need to iterate the Layout's views.
	parent = ChildAt(0);
#endif

	for (int32 i = 0; i < parent->CountChildren(); i++) {
		BView *child = parent->ChildAt(i);
		
		BMenu *menu = dynamic_cast<BMenu *>(child);
		BTextControl *textcontrol = dynamic_cast<BTextControl *>(child);
		NotifyingTextView *textview = dynamic_cast<NotifyingTextView *>(child);
		BCheckBox *checkbox = dynamic_cast<BCheckBox *>(child);
		BMenuField *menufield = dynamic_cast<BMenuField *>(child);
				
		if (menufield != NULL) {
			menu = menufield->Menu();
		};
		if (menu != NULL) {
			for (int32 j = 0; j < menu->CountItems(); j++) {
				BMenuItem *item = menu->ItemAt(j);
				item->SetMessage(new BMessage(kMsgControlChanged));
				item->SetTarget(parent);
			};
		
			menu->SetTargetForItems(parent);
		};
		
		if (textcontrol != NULL) {
			textcontrol->SetMessage(new BMessage(kMsgControlChanged));
			textcontrol->SetTarget(parent);
		};
		
		if (checkbox != NULL) {
			checkbox->SetMessage(new BMessage(kMsgControlChanged));
			checkbox->SetTarget(parent);
		};
		
		if (textview != NULL) {
			textview->SetHandler(this);
			textview->SetNotificationMessage(BMessage(kMsgControlChanged));
		};
	};
};

void PClientView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kMsgControlChanged: {
			if (fHost != NULL) {
				fHost->ControllerModified(this);
			};
		} break;
		default: {
			BView::MessageReceived(msg);
		} break;
	};
}; 

void PClientView::Draw(BRect) {
	SetPenSize(2.0f);
	StrokeRect(Bounds());
}

void PClientView::GetPreferredSize(float *width, float *height) {
	*width = Frame().Width();
	*height = fRequiredHeight;
};

//#pragma mark SettingsController Hooks

status_t PClientView::Init(SettingsHost *host) {
	fHost = host;
	return B_OK;
};

status_t PClientView::Save(const BMessage *tmplate, BMessage *settings) {
	return SaveSettings(this, *tmplate, settings);
};

status_t PClientView::Revert(const BMessage *tmplate) {
	return B_OK;
};

//#pragma mark Public

bool PClientView::ShowHeading(void) const {
	return fShowHeading;
};

//#pragma mark Private

float PClientView::BuildGUI(void) {
	BMessage curr;
	float inset = ceilf(be_plain_font->Size() * 0.7);

#ifdef __HAIKU__
	// Setup layout
	SetLayout(new BGroupLayout(B_VERTICAL));
	BGroupLayoutBuilder layout(B_VERTICAL, inset);
#else
	float yOffset = kEdgeOffset + kControlOffset;
	float xOffset = 0;

	font_height fontHeight;
	be_plain_font->GetHeight(&fontHeight);

	const float kControlWidth = Bounds().Width() - (kEdgeOffset * 2);
	const float kFontHeight = fontHeight.descent + fontHeight.leading + fontHeight.ascent;
#endif

	if (fShowHeading) {
		BFont headingFont(be_bold_font);
		headingFont.SetSize(headingFont.Size() * 1.2f);

		BStringView* descLabel = new BStringView(BRect(0, 0, 1, 1), fTitle.String(), fTitle.String());
		descLabel->SetFont(&headingFont);
		descLabel->SetAlignment(B_ALIGN_LEFT);

		Divider *divider = new Divider(Frame(), "DescDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
		divider->ResizeToPreferred();

#ifdef __HAIKU__
		descLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
		divider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));

		layout.Add(descLabel);
		layout.Add(divider);
#else
		descLabel->ResizeToPreferred();	

		AddChild(descLabel);
		AddChild(divider);

		BRect frameDescLabel = descLabel->Frame();
		BRect frameDivider = divider->Frame();

		divider->MoveTo(frameDivider.left, frameDescLabel.bottom + inset);
		frameDivider = divider->Frame();

		yOffset = frameDivider.bottom + inset;
#endif
	};

	for (int32 i = 0; fTemplate.FindMessage("setting", i, &curr) == B_OK; i++) {
		char temp[512];

		// Get stuff from settings template
		const char *name = curr.FindString("name");
		const char *desc = curr.FindString("description");
		const char *value = NULL;
		int32 type = -1;
		bool secret = false;
		bool freeText = true;
		bool multiLine = false;
		BView *control = NULL;
		BMenu *menu = NULL;
		BRect frame(0, 0, 1, 1);


		if (name != NULL && strcmp(name,"app_sig") == 0) {
			// skip app-sig setting
			continue;
		}

		if (curr.FindInt32("type", &type) != B_OK) {
			LOG("preflet", liMedium, "Error getting type for %s, skipping", name);
			continue;
		}

		switch (type) {
			case B_STRING_TYPE: {
				if (curr.FindString("valid_value")) {
					// It's a "select one of these" setting
					freeText = false;

					menu = new BPopUpMenu(name);
					for (int j = 0; curr.FindString("valid_value", j); j++) {
						menu->AddItem(new BMenuItem(curr.FindString("valid_value", j), NULL));
					};

					value = fSettings.FindString(name);
					if (value) {
						menu->FindItem(value)->SetMarked(true);
					};
				} else {
					// It's a free-text setting
					if (curr.FindBool("multi_line", &multiLine) != B_OK) {
						multiLine = false;
					};
					value = fSettings.FindString(name);
					if (!value) {
						value = curr.FindString("default");
					};
					if (curr.FindBool("is_secret",&secret) != B_OK) {
						secret = false;
					};
				}
			} break;
			case B_INT32_TYPE: {
				if (curr.FindInt32("valid_value")) {
					// It's a "select one of these" setting

					freeText = false;

					menu = new BPopUpMenu(name);

					int32 def = 0;
					status_t hasValue = fSettings.FindInt32(name, 0, &def);

					if (hasValue != B_OK) {
						hasValue = curr.FindInt32("default", 0, &def);
					};

					int32 v = 0;
					for (int j = 0; curr.FindInt32("valid_value",j,&v) == B_OK; j++) {
						sprintf(temp,"%ld", v);

						BMenuItem* item = new BMenuItem(temp, NULL);

						if (hasValue != B_OK && j == 0) {
							item->SetMarked(true);
						} else if ((hasValue == B_OK) && (def == v)) {
							item->SetMarked(true);
						};

						menu->AddItem(item);
					}
				} else {
					// It's a free-text (but number) setting
					int32 v = 0;
					if (fSettings.FindInt32(name, &v) == B_OK) {
						sprintf(temp,"%ld", v);
						value = temp;
					} else if (curr.FindInt32("default", &v) == B_OK) {
						sprintf(temp,"%ld", v);
						value = temp;
					}
					if (curr.FindBool("is_secret",&secret) != B_OK) {
						secret = false;
					};
				}
			} break;
			case B_BOOL_TYPE: {
				bool active;

				if (fSettings.FindBool(name, &active) != B_OK) {
					if (curr.FindBool("default", &active) != B_OK) {
						active = false;
					}
				}

#ifndef __HAIKU__
				frame = BRect(0, 0, kControlWidth, kFontHeight);
#endif
				control = new BCheckBox(frame, name, _T(desc), NULL);
				if (active) {
					((BCheckBox*)control)->SetValue(B_CONTROL_ON);
				};
			} break;
			default:
				continue;
		}

		if (!value) {
			value = "";
		};

		if (!control) {
			if (freeText) {
				if (!multiLine) {
#ifndef __HAIKU__
					frame = BRect(0, 0, kControlWidth, kFontHeight);
#endif
					control = new BTextControl(frame, name, _T(desc), value, NULL);
					if (secret) {
						((BTextControl*)control)->TextView()->HideTyping(true);
						((BTextControl*)control)->SetText(_T(value));
					}
					((BTextControl*)control)->SetDivider(kDividerWidth);
				} else {
#ifndef __HAIKU__
					frame  = BRect(0, 0, kDividerWidth, kFontHeight);
#endif
					BStringView* label = new BStringView(frame, "NA", _T(desc), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW);
#ifdef __HAIKU__
					layout.Add(label);
#else
					AddChild(label);
					label->MoveTo(kEdgeOffset, yOffset);

					frame = BRect(0, 0, kControlWidth - kDividerWidth, kFontHeight * 4);
					frame.right -= B_V_SCROLL_BAR_WIDTH + kEdgeOffset + kControlOffset;

					xOffset = kEdgeOffset + kDividerWidth;
#endif

					BRect textRect = frame;
					textRect.InsetBy(kEdgeOffset, kEdgeOffset);
					textRect.OffsetTo(1.0, 1.0);

					NotifyingTextView *textView = new NotifyingTextView(frame, name, textRect, B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

					control = new BScrollView("NA", textView, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_NAVIGABLE, false, true);
					textView->SetText(_T(value));			
				}
			} else {
#ifndef __HAIKU__
				frame = BRect(0, 0, kControlWidth, kFontHeight);
#endif
				control = new BMenuField(frame, name, _T(desc), menu);
				((BMenuField *)control)->SetDivider(kDividerWidth);
			}
		}

		if (curr.FindString("help")) {
			gHelper.SetHelp(control, strdup(curr.FindString("help")));
		};

#ifdef __HAIKU__
		layout.Add(control);
#else
		AddChild(control);

		float h, w = 0;
		control->GetPreferredSize(&w, &h);
		control->MoveTo(kEdgeOffset + xOffset, yOffset);

		yOffset += kControlOffset + h;
		xOffset = 0;
#endif
	};

#ifdef __HAIKU__
	layout.AddGlue();
	AddChild(layout);

	return 0.0f;
#else
	if (yOffset < Bounds().Height())
		yOffset = Bounds().Height();

	return yOffset;
#endif
};
