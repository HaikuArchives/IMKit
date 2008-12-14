/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PACCOUNT_DIALOG_H
#define PACCOUNT_DIALOG_H

#include <interface/Window.h>

class BPath;
class BTextControl;

class PAccountDialog : public BWindow
{
	public:
						PAccountDialog(const char* title, BPath* addonPath);

		virtual void	MessageReceived(BMessage* msg);

		int32			Go();

		const char*		AccountName();

	private:
		sem_id			fSem;
		int32			fValue;
		BTextControl*	fAccountName;
};

#endif // PACCOUNT_DIALOG_H
