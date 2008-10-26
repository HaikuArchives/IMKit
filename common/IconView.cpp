#include "IconView.h"

#include <stdio.h>

#include <Bitmap.h>
#include <Path.h>

#include "IMKitUtilities.h"

//#pragma mark Constructor
IconView::IconView(BBitmap *icon)
	: BView(icon->Bounds(), "IconView", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW),
	fBitmap(icon),
	fEnabled(true) {
};

IconView::IconView(entry_ref ref, int16 size = kSmallIcon, bool followSymlink = true)
	: BView(BRect(1, 1, size, size), "IconView", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW),
	fBitmap(NULL),
	fEnabled(true) {
	
	BPath path(&ref);
	fBitmap = ReadNodeIcon(path.Path(), size, followSymlink);
};

IconView::~IconView(void) {
	delete fBitmap;
};

//#pragma mark Hooks

void IconView::Draw(BRect frame) {
	PushState();

		SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		FillRect(Bounds());

		if ((fEnabled) && (fBitmap)) {
			SetDrawingMode(B_OP_ALPHA);
			DrawBitmap(fBitmap);			
		};

	PopState();
};

void IconView::AttachedToWindow(void) {
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
 	SetHighColor(0, 0, 0, 255);
};

//#pragma mark Public
void IconView::EnableDrawing(bool enable) {
	fEnabled = enable;
};

bool IconView::DrawingEnabled(void) {
	return fEnabled;
};

