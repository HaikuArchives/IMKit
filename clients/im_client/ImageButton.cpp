#include "ImageButton.h"

#include <string.h>
#include <stdio.h>

ImageButton::ImageButton( BRect rect, const char * name, BMessage * msg, uint32 resizing, uint32 flags, BBitmap * bitmap, const char * label )
:	BControl(rect,name,label,msg,resizing,flags)
{
	fBitmap = bitmap;
	fMouseDown = false;
	fMouseOver = false;
	
	if ( !fBitmap )
		printf("ImageButton: fImage null\n");
}

ImageButton::~ImageButton()
{
	delete fBitmap;
//	free(fText);
}

void
ImageButton::MouseDown( BPoint /*where*/ )
{
	SetMouseEventMask( B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS );
	
	fMouseDown = true;
	
	Invalidate();
}

void
ImageButton::MouseMoved( BPoint /*where*/, uint32 transition, const BMessage * /*msg*/ )
{
	switch ( transition )
	{
		case B_ENTERED_VIEW:
			fMouseOver = true;
			Invalidate();
			break;
		case B_EXITED_VIEW:
			fMouseOver = false;
			Invalidate();
			break;
	}
}

void
ImageButton::MouseUp( BPoint where )
{
	if ( fMouseDown && Bounds().Contains(where) )
	{ // clicked
		Invoke();
	}
	
	fMouseDown = false;
	
	Invalidate();
}

void
ImageButton::Draw( BRect /*update_rect*/ )
{
	rgb_color view_color = ViewColor();
	
	rgb_color highlight = tint_color( view_color, B_DARKEN_1_TINT );
	rgb_color light = tint_color( view_color, B_LIGHTEN_2_TINT );
	rgb_color dark = tint_color( view_color, B_DARKEN_2_TINT );
	
	rgb_color first, second;
	
	if ( fMouseDown )
	{
		SetHighColor( highlight );
		FillRect( Bounds() );
	}
	
	if ( fMouseOver )
	{
		first = light;
		second = dark;
	}
	
	if ( fMouseDown )
	{
		first = dark;
		second = light;
	}
	
	if ( fMouseOver || fMouseDown )
	{
		MovePenTo( 0, Bounds().bottom );

		SetHighColor( first );
		StrokeLine( BPoint(0,0) );
		StrokeLine( BPoint(Bounds().right, 0) );
		
		SetHighColor( second );
		StrokeLine( BPoint(Bounds().right, Bounds().bottom) );
		StrokeLine( BPoint(0, Bounds().bottom) );
	}
	
	SetHighColor(0,0,0);
	
	float center = Bounds().Width() / 2;
	
	if (fBitmap != NULL) {
		float image_width = fBitmap->Bounds().Width();
	
		SetDrawingMode( B_OP_ALPHA );
		DrawBitmap( fBitmap, BPoint( center - image_width/2, 4.0 ) );
	}
	
	if ( Label() )
	{
		float str_width = StringWidth( Label() );
		
		DrawString( 
			Label(), 
			BPoint( center - str_width/2, 
			Bounds().bottom - 3) 
		);
	}
}
