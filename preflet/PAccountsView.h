/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PACCOUNTS_VIEW_H
#define PACCOUNTS_VIEW_H

#include <interface/View.h>

class BPath;
class BWindow;
class BButton;

class PAccountsView : public BView
{
	public:
				PAccountsView(BRect bounds, BPath* protoPath);

		virtual void	AttachedToWindow();
		virtual void	MessageReceived(BMessage* msg);

	private:
		BPath*		fProtoPath;
		BButton*	fAddButton;
		BButton*	fEditButton;
		BButton*	fDelButton;
};

#endif // PACCOUNTS_VIEW_H
