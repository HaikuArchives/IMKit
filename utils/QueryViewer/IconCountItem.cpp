#include "IconCountItem.h"

#include <stdio.h>

const float kEdgeOffset = 2.0;
const rgb_color kHighlight = {140, 140, 140, 255};

IconCountItem::IconCountItem(const char *text, const char *path,
	BBitmap *icon, bool isNew) 
	: fIcon(NULL),
	fIconHeight(0),
	fIconWidth(0) {

	fNew = isNew;
	fText = text;
	fIcon = icon;
	fPath = path;
	fCount = 0;
};

IconCountItem::~IconCountItem(void) {
	delete fIcon;
};

void IconCountItem::DrawItem(BView *owner, BRect frame, bool complete) {
	if (IsSelected() || complete) {
		rgb_color color;
		rgb_color origHigh;
		
		origHigh = owner->HighColor();
		
		if (IsSelected()) {
			color = kHighlight;
		} else {
			color = owner->ViewColor();
		};
		
		owner->SetHighColor(color);
		owner->FillRect(frame);
		owner->SetHighColor(origHigh);
	}
	
	if (fIcon) {
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmap(fIcon, BPoint(frame.left + kEdgeOffset,
			frame.top + kEdgeOffset));
		owner->SetDrawingMode(B_OP_COPY);
	};
	
	if (IsSelected()) owner->SetDrawingMode(B_OP_OVER);

	owner->MovePenTo(frame.left + kEdgeOffset + fIconWidth + kEdgeOffset,
		frame.bottom - fFontOffset);
		
	BString text = fText;
	if (Count() > 0) text << " (" << Count() << ")";

	if (IsNew()) {
		owner->PushState();
		owner->SetHighColor(255, 0, 0);
	};
	owner->DrawString(text.String());

	if (IsNew()) owner->PopState();
};

const char *IconCountItem::Text(void) const {
	return fText.String();
};

const BBitmap *IconCountItem::Icon(void) const {
	return fIcon;
};

void IconCountItem::Update(BView */*owner*/, const BFont *font) {
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.descent + fontHeight.leading + fontHeight.ascent;
	
	if (fIcon) {
		fIconHeight = fIcon->Bounds().Height() + 1;
		fIconWidth = fIcon->Bounds().Width();

		if (fIconHeight > fFontHeight) {
			SetHeight(fIconHeight + (kEdgeOffset * 2));
			fFontOffset = ((fIconHeight - fFontHeight) / 2) + (kEdgeOffset * 2);
		} else {
			SetHeight(fFontHeight + (kEdgeOffset * 2));
			fFontOffset = kEdgeOffset;
		};
	} else {
		SetHeight(fFontHeight + (kEdgeOffset * 2));
		fFontOffset = kEdgeOffset;
	};	
};

void IconCountItem::Count(int32 count) {
	fCount = count;
};

int32 IconCountItem::Count(void) {
	return fCount;
};

bool IconCountItem::IsNew(void) {
	return fNew;
};

void IconCountItem::IsNew(bool isnew) {
	fNew = isnew;
};

const char *IconCountItem::Path(void) {
	return fPath.String();
};
