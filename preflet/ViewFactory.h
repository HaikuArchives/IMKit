/*
 * Copyright 2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 */
 
#ifndef VIEWFACTORY_H
#define VIEWFACTORY_H

#include <interface/Rect.h>

class BBox;
class BView;

class ViewFactory {
	public:

		// Public Methods
		template <class T>
		static T		*Create(BRect rect, const char *name, uint32 resize, uint32 flags);
};

#endif // VIEWFACTORY_H
