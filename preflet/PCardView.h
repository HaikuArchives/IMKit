/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PCARDVIEW_H
#define PCARDVIEW_H

#include "ViewFactory.h"

class PCardView : public AbstractView {
	public:
			PCardView(BRect bounds);

		void	Add(BView *view);
		void	Select(BView *view);

	private:
		BView*	fCurrentView;
};

#endif // PCARDVIEW_H
