/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PVIEW_H
#define PVIEW_H

#include <interface/View.h>
#include <interface/Button.h>

class BOutlineListView;
class BBox;

class PView : public BView
{
	public:
					PView();

	private:
		BOutlineListView*	fListView;
		BBox*				fBox;
		BButton*			fRevert;
		BButton*			fSave;
};

#endif // PVIEW_H
