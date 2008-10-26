#ifndef ICONVIEW_H
#define ICONVIEW_H

#include <Entry.h>
#include <View.h>

class BBitmap;

extern const int32 kSmallIcon;

class IconView : public BView {
	public:
							IconView(BBitmap *icon);
							IconView(entry_ref ref, int16 size = kSmallIcon,
								bool followSymlink = true);
							
							~IconView(void);

		// Hooks
		void				Draw(BRect frame);
		void				AttachedToWindow(void);
		
		// Public
		void				EnableDrawing(bool enable);
		bool				DrawingEnabled(void);
		
	private:
	
		BBitmap				*fBitmap;
		bool				fEnabled;
};

#endif
