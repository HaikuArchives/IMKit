#include "ImageButton.h"

#include <string.h>
#include <stdio.h>
#include "StdBevels.h"

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
		case B_INSIDE_VIEW:
			if(!fMouseOver){
				fMouseOver=true;
				Invalidate();
			}
			break;
		case B_OUTSIDE_VIEW:
			if(fMouseOver){
				fMouseOver=false;
				Invalidate();
			}
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
ImageButton::Draw( BRect /* update_rect */ )
{
	if ( fMouseDown )
	{
		SetHighColor( tint_color( ViewColor(), B_DARKEN_1_TINT ) );
		FillRect( Bounds() );
		StdBevels::DrawBorderBevel(this, Bounds(), StdBevels::DEPRESSED_BEVEL);

	}
	else
	if ( fMouseOver )
	{	
		StdBevels::DrawBorderBevel(this,  Bounds(), StdBevels::NORMAL_BEVEL);
	
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
