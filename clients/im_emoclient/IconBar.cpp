#include "IconBar.h"

IconBar::IconBar(BRect rect)
	: BView(rect, "statusbar", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW) {
	
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
 	SetLowColor (216, 216, 216, 255);
 	SetHighColor (0, 0, 0, 255);
};

IconBar::~IconBar() {
};

void IconBar::MessageReceived(BMessage *msg) {
	BView::MessageReceived(msg);
};

int32 IconBar::AddItem(BView *view) {
	fViews.AddItem(view);
	AddChild(view);
	PositionViews();
	return fViews.CountItems();
};

BView *IconBar::ViewAt(int32 index) {
	return (BView *)fViews.ItemAt(index);
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

void IconBar::PositionViews() {
	float currw = 2;
	
	for ( int32 i=0; i<fViews.CountItems(); i++ ) {
		BView * view = ViewAt(i);
		
		view->MoveTo( currw, 3 );
		view->Invalidate();
		
		currw = view->Frame().right + 4;
	}
	
	Invalidate();
}
