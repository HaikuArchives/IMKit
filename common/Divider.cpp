
#include <app/Message.h>

#include "Divider.h"

//#pragma mark Constants

const float kResizeWidgetRatio = 0.1f;
const float kResizeWidgetPadding = 2.0f;
const float kResizeWidgetCircleRadius = 2.0f;
const float kResizeWidgetSpacing = 10.0f;
const int kResizeWidgetCircleCount = 2;

//#pragma mark Constructor

Divider::Divider(BRect frame, const char *name, uint32 resize, uint32 flags) :
	BView(frame, name, resize, flags | B_FRAME_EVENTS | B_WILL_DRAW),
	fOrient(B_HORIZONTAL) {
	
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
};
 
Divider::Divider(BMessage *archive)
	: BView(archive) {
	
	if (archive->FindInt32("orientation", (int32 *)&fOrient) != B_OK) fOrient = B_HORIZONTAL;
};
 
Divider::~Divider() {
};

//#pragma mark BView Hooks

void Divider::Draw(BRect updateRect) {
	BRect bounds = Bounds();

	rgb_color view_color = ViewColor();
	
	rgb_color line = tint_color(view_color, B_DARKEN_1_TINT);
	rgb_color shadow = tint_color(view_color, B_DARKEN_2_TINT);

	PushState();

		SetHighColor(view_color);
		FillRect(bounds);
	
		if (fOrient == B_HORIZONTAL) {
			BPoint left(bounds.left, (bounds.Height() / 2) + bounds.top);
			BPoint right(bounds.right, left.y);
			
			BPoint fudge(PenSize(), PenSize());

			left -= fudge;
			right -= fudge;
			SetHighColor(line);
			StrokeLine(left, right);
			
			left += fudge;
			right += fudge;
			SetHighColor(shadow);
			StrokeLine(left, right);
		} else {
			BPoint top((bounds.Width() / 2) + bounds.left, bounds.top);
			BPoint bottom(top.x, bounds.bottom);
			
			BPoint fudge(PenSize(), PenSize());
			
			top -= fudge;
			bottom -= fudge;		
			SetHighColor(line);
			StrokeLine(top, bottom);
			
			top += fudge;
			bottom += fudge;
			SetHighColor(shadow);
			StrokeLine(top, bottom);
		};
		
	PopState();
};

void Divider::GetPreferredSize(float *width, float *height) {
	if (fOrient == B_HORIZONTAL) {
		*width = Bounds().Width();
		*height = (PenSize() * 4);	// Two widths for padding (either side) + line + shadow
	} else {
		*height = Bounds().Height();
		*width = (PenSize() * 4);	// Two widths for padding (either side) + line + shadow
	};
};

//#pragma BArchivable Hooks

status_t Divider::Archive(BMessage *archive, bool deep) const {
	archive->AddInt32("orientation", fOrient);
	return BView::Archive(archive, false);
};

BArchivable *Divider::Instantiate(BMessage *archive) {
	BArchivable *instance = NULL;

	if (validate_instantiation(archive, "Divider") == true) {
		instance = new Divider(archive);
	};

	return instance;

};

//#pragma mark Public

orientation Divider::Orientation(void) {
	return fOrient;
};

void Divider::Orientation(orientation orient) {
	fOrient = orient;
};

//#pragma mark Private
