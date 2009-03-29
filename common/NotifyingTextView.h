#ifndef COMMON_NOTIFYINGTEXTVIEW_H
#define COMMON_NOTIFYINGTEXTVIEW_H

/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 */

#include <app/Handler.h>
#include <app/Message.h>
#include <interface/TextView.h>

class BMessenger;

class NotifyingTextView : public BTextView {
	public:
	
							NotifyingTextView(BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags);
							~NotifyingTextView(void);
		
		void				SetHandler(BHandler *handler);
		
		BMessage			NotificationMessage(void) const;
		void				SetNotificationMessage(BMessage msg);
		
	protected:
	
		// BTextView Hooks
		virtual void		InsertText(const char *text, int32 length, int32 offset, const text_run_array *runs = NULL);
		virtual void		DeleteText(int32 start, int32 finish);
		
	private:
		BMessenger			*fMessenger;
		BMessage			fMessage;
		
};

#endif
