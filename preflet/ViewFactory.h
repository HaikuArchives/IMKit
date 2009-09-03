/*
 * Copyright 2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */
 
#ifndef VIEWFACTORY_H
#define VIEWFACTORY_H

#include <Message.h>
#include <interface/View.h>

class ViewFactory {
	public:
		template <class T>
		static T		*Create(BRect rect, const char *name, uint32 resize, uint32 flags);

		template <class T>
		static T		*Create(BRect rect, const char *name, const char *label, BMessage *msg);
};

class AbstractView : public BView {
	public:
				AbstractView(BRect frame, const char *name, uint32 resizeMask, uint32 flags);

		virtual void	AttachedToWindow(void);
};

#endif // VIEWFACTORY_H
