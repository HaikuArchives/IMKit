#include "StatusBar.h"

StatusBar::StatusBar(BRect rect)
	: BView(rect, "statusbar", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW) {
	
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
 	SetLowColor (216, 216, 216, 255);
 	SetHighColor (0, 0, 0, 255);
};

StatusBar::~StatusBar() {
};

void StatusBar::MessageReceived(BMessage *msg) {
	BView::MessageReceived(msg);
};

int32 StatusBar::AddItem(BView *view) {
	fViews.AddItem(view);
	AddChild(view);
	return fViews.CountItems();
};

BView *StatusBar::ViewAt(int32 index) {
	return (BView *)fViews.ItemAt(index);
};

void StatusBar::Draw(BRect update) {
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
	
/*	for (int32 i = 0; i < fViews.CountItems(); ++i) {
		if (i) {
			DrawSplitter(width += 3);
			width += 5;
	    };
	    
	    BView *view = ViewAt(i);
	    if (view) {
			float w, h;
			view->GetPreferredSize(&w, &h);
			width = w;
	    }
	};*/
	for ( int32 i=0; i<fViews.CountItems()-1; i++ ) {
		DrawSplitter( ViewAt(i)->Frame().right + 3 );
	}
}

void StatusBar::DrawSplitter(float x) {
	BRect bounds = Bounds();

	PushState();

	SetDrawingMode(B_OP_COPY);
  
	SetHighColor(131, 131, 131, 255);
	StrokeLine(BPoint(x, bounds.top + 2.0), BPoint(x, bounds.bottom));

	SetHighColor(255, 255, 255, 255);
	StrokeLine(BPoint(x + 1, bounds.top + 2.0), BPoint(x + 1, bounds.bottom));

	PopState();
};

void StatusBar::PositionViews() {
	float currw = 0;
	
	for ( int32 i=0; i<fViews.CountItems(); i++ ) {
		BView * view = ViewAt(i);
		
		view->MoveTo( currw, view->Frame().top );
		view->Invalidate();
		
		currw = view->Frame().right + 6;
	}
	
	Invalidate();
}
