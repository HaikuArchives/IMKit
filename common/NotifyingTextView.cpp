#include "NotifyingTextView.h"

/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *	Michael Davidson <slaad@bong.com.au>
 */

#include <app/Messenger.h>

//#pragma mark Constructor

#ifdef __HAIKU__
NotifyingTextView::NotifyingTextView(const char *name, uint32 flags)
	: BTextView(name, flags),
	fMessenger(NULL) {
};
#endif

NotifyingTextView::NotifyingTextView(BRect frame, const char *name, uint32 resizeMask, uint32 flags)
	: BTextView(frame, name, BRect(0, 0, 1, 1), resizeMask, flags),
	fMessenger(NULL) {
};

NotifyingTextView::~NotifyingTextView(void) {
	if (fMessenger != NULL) delete fMessenger;
};

//#pragma mark Public

void NotifyingTextView::SetHandler(BHandler *handler) {
	if (fMessenger != NULL) delete fMessenger;
	
	fMessenger = new BMessenger(handler);
};

BMessage NotifyingTextView::NotificationMessage(void) const {
	return fMessage;
};

void NotifyingTextView::SetNotificationMessage(BMessage msg) {
	fMessage = msg;
}
		
//#pragma mark BTextView Hooks

void NotifyingTextView::InsertText(const char *text, int32 length, int32 offset, const text_run_array *runs = NULL) {
	if ((fMessenger != NULL) && (fMessenger->IsValid() == true)) {
		BMessage msg(fMessage);
		msg.AddPointer("source", this);
		
		fMessenger->SendMessage(&msg);
	};
	
	BTextView::InsertText(text, length, offset, runs);
};

void NotifyingTextView::DeleteText(int32 start, int32 finish) {
	if ((fMessenger != NULL) && (fMessenger->IsValid() == true)) {
		BMessage msg(fMessage);
		msg.AddPointer("source", this);
		
		fMessenger->SendMessage(&msg);
	};
	
	BTextView::DeleteText(start, finish);
};

