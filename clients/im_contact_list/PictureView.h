/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PICTURE_VIEW_H
#define _PICTURE_VIEW_H

#include <interface/View.h>

class BBitmap;

class PictureView : public BView {
public:
						PictureView(const char* filename, uint32 resizing_flags
							= B_FOLLOW_H_CENTER, uint32 flags = B_WILL_DRAW);
						~PictureView();

	virtual BSize		MinSize();
	virtual BSize		MaxSize();
	virtual BSize		PreferredSize();

	virtual	void		Draw(BRect frame);	

			status_t	InitCheck();

private:
			BBitmap*			fBitmap;
			float				fWidth;
			float				fHeight;
};

#endif	// _PICTURE_VIEW_H
