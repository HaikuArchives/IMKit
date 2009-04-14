#ifndef IMAGE_BUTTON_H
#define IMAGE_BUTTON_H

#include <Control.h>
#include <Bitmap.h>

class ImageButton : public BControl
{
	public:
		ImageButton( BRect rect, const char * name, BMessage * msg, uint32 resizing, uint32 flags, BBitmap * bitmap, const char * label );
		virtual ~ImageButton();
		
		virtual void MouseDown( BPoint );
		virtual void MouseMoved( BPoint, uint32, const BMessage * );
		virtual void MouseUp( BPoint );
		
		virtual void Draw( BRect );
	
	private:
		bool		fMouseDown, fMouseOver;
		BBitmap		* fBitmap;
};

#endif
