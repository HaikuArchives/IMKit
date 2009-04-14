#include "IconBar.h"

#include <Directory.h>
#include <Entry.h>
#include <Node.h>
#include <Path.h>
#include <Roster.h>

#include <common/BubbleHelper.h>
#include "ImageButton.h"
#include <common/IMKitUtilities.h>

#include <stdio.h>

//#pragma mark Constants

const int32 kClickMsg = 'ib01';

//#pragma mark Constructor

IconBar::IconBar(BRect rect, const char *path, BubbleHelper *helper, int16 padding,
	entry_ref ref)
	: BView(rect, "IconBar", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW),
	fContactRef(ref),
	fAppPath(path),
	fPadding(padding),
	fBubbles(helper) {
	
//	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
// 	SetLowColor (216, 216, 216, 255);
// 	SetHighColor (0, 0, 0, 255);
//
//	BDirectory dir(path);
// 	entry_ref ref;
//	int16 iconSize = (int16)(rect.Height() - (padding * 2));
// 	BRect iconRect = rect;
// 	
//	iconRect.bottom -= padding;
//	iconRect.top += padding;
// 	iconRect.right = iconSize;
// 	iconRect.left += padding;
// 	
// 	iconSize -= padding * 2;
//	
// 	while (dir.GetNextRef(&ref) == B_OK) {
// 		BPath path(&ref);
// 		BNode node(&ref);
// 		BBitmap *icon = ReadNodeIcon(path.Path(), iconSize, true);
//
//		int32 length = -1;
//		char *desc = ReadAttribute(node, "im_client:description", &length);
//		if ((length < 1) || (desc == NULL)) desc = strdup(ref.name);
//		
//		BMessage *msg = new BMessage(kClickMsg);
//		msg->AddRef("app_ref", &ref);
//				
// 		ImageButton *button = new ImageButton(iconRect, ref.name, msg,
// 			B_FOLLOW_NONE, B_WILL_DRAW, icon, NULL);
// 		helper->SetHelp(button, desc);
// 		AddChild(button);
// 		
// 		button->SetTarget(this);
// 		
// 		fButtons.push_back(button);
// 		
// 		free(desc);
//
// 		
// 		iconRect.OffsetBy(iconRect.Width() + padding, 0);
// 	};
};

IconBar::~IconBar() {
	int32 buttons = fButtons.size();
	for (int32 i = 0; i < buttons; i++) {
		ImageButton *button = fButtons[i];
		button->RemoveSelf();
		delete button;
	};
};

//#pragma mark Hooks

void IconBar::AttachedToWindow(void) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
 	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
 	SetHighColor(0, 0, 0, 255);

	BDirectory dir(fAppPath.String());
 	entry_ref ref;
 	BRect iconRect = Bounds();

	fIconSize = (int16)(iconRect.Height() - (fPadding * 2));
 	
	iconRect.bottom -= fPadding;
	iconRect.top += fPadding;
 	iconRect.right = fIconSize;
 	iconRect.left += fPadding;
 	
 	fIconSize -= fPadding * 4;
	
 	while (dir.GetNextRef(&ref) == B_OK) {
 		BPath path(&ref);
 		BNode node(&ref);
 		BBitmap *icon = ReadNodeIcon(path.Path(), fIconSize, true);

		int32 length = -1;
		char *desc = ReadAttribute(node, "im_client:description", &length);
		if ((length < 1) || (desc == NULL)) desc = strdup(ref.name);
		
		BMessage *msg = new BMessage(kClickMsg);
		msg->AddRef("app_ref", &ref);
				
 		ImageButton *button = new ImageButton(iconRect, ref.name, msg,
 			B_FOLLOW_NONE, B_WILL_DRAW, icon, NULL);
 		fBubbles->SetHelp(button, desc);
 		AddChild(button);
 		
 		button->SetTarget(this);
 		
 		fButtons.push_back(button);
 		
 		free(desc);

 		
 		iconRect.OffsetBy(iconRect.Width() + fPadding, 0);
 	};
};

void IconBar::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kClickMsg: {
			entry_ref appRef;
			if (msg->FindRef("app_ref", &appRef) != B_OK) return;
			
			BMessage click(B_REFS_RECEIVED);
			click.AddRef("refs", &fContactRef);
			
			be_roster->Launch(&appRef, &click);
		} break;
		default: {
			BView::MessageReceived(msg);
		};
	};
};

void IconBar::Draw(BRect update) {
	SetDrawingMode(B_OP_COPY);
	SetHighColor(131, 131, 131, 255);
	StrokeLine(BPoint(update.left, Bounds().top),
		BPoint(update.right, Bounds().top));
	SetHighColor(255, 255, 255, 255);
	StrokeLine(BPoint(update.left, Bounds().top + 1),
		BPoint(update.right, Bounds().top + 1));
	
//	float width (5.0);
	font_height fh;
	
	GetFontHeight (&fh);
	
	SetDrawingMode(B_OP_OVER);
	SetHighColor(0, 0, 0, 255);
}

