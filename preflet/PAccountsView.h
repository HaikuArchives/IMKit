/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PACCOUNTS_VIEW_H
#define PACCOUNTS_VIEW_H

#include <interface/View.h>

class BPath;
class BButton;

class PAccountsView : public BView
{
	public:
				PAccountsView(BRect bounds, BPath* protoPath);

		virtual void	AttachedToWindow();
		virtual void	MessageReceived(BMessage* msg);

	private:
		BButton*	fAddButton;
		BButton*	fEditButton;
		BButton*	fDelButton;
		BButton*	fCancelButton;
		BButton*	fOkButton;
};

#endif // PACCOUNTS_VIEW_H
