/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _AWAY_MESSAGE_WINDOW_H
#define _AWAY_MESSAGE_WINDOW_H

#include <Window.h>
#include <String.h>

#include <libim/AccountInfo.h>

class BButton;
class BScrollView;
class BTextView;
class BView;

class AwayMessageWindow : public BWindow {
public:
						AwayMessageWindow(IM::AccountInfo* info = NULL);
	virtual				~AwayMessageWindow();

	// BWindow Hooks
	virtual	bool		QuitRequested();
	virtual	void 		MessageReceived(BMessage* msg);

private:	
	BView*				fView;
	BTextView*			fTextView;
	BScrollView*		fScroller;
	float				fFontHeight;
	BButton*			fOkay;
	BButton*			fCancel;
	BString				fAccountInstance;
};

#endif	// _AWAY_MESSAGE_WINDOW_H
