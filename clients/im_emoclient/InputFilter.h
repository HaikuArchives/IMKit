#ifndef INPUTFILTER_H
#define INPUTFILTER_H

#include <MessageFilter.h>
#include <Message.h>
#include <TextView.h>
#include <Window.h>

class BMessageFilter;

class InputFilter : public BMessageFilter {
	public:
								InputFilter(BTextView *owner, BMessage *msg,
									bool commandSends, BView * forward_to,
									int32 interval);
	    virtual filter_result	Filter (BMessage *, BHandler **);
    	filter_result			HandleKeys (BMessage *);
    
	private:
		BTextView	 			*fParent;
		BView					*fForwardTo;
		BMessage				*fMessage;
		bigtime_t				fLastTyping;
		bool					fCommandSends;
		int32					fInterval;
};

#endif
