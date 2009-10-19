#include "InputFilter.h"

#include <libim/Constants.h>
#include <Messenger.h>

#include <stdio.h>
#include <string.h>

InputFilter::InputFilter (BTextView *owner, BMessage *msg, bool commandSends,
	BView * forward_to, int32 interval) 
	: BMessageFilter (B_ANY_DELIVERY, B_ANY_SOURCE),
	fParent(owner),
	fForwardTo(forward_to),
	fMessage(new BMessage(*msg)),
	fLastTyping(0),
	fCommandSends(commandSends),
	fInterval(interval) {
}

filter_result InputFilter::Filter (BMessage *msg, BHandler ** /*target*/) {

	filter_result result (B_DISPATCH_MESSAGE);

	switch (msg->what) {
		case B_MOUSE_MOVED: {
		} break;
		
		case B_MOUSE_WHEEL_CHANGED: {
			BMessenger(fParent->Window()).SendMessage(msg);
			return B_SKIP_MESSAGE;
		} break;
		
		case B_KEY_DOWN: {
			result = HandleKeys (msg);
		} break;

		case B_COPY: {
			// copy requested
			int32 start, finish;
			
			fParent->GetSelection(&start,&finish);
			
			if ( start == finish )
			{ // nothing selected, forward message to history view
				BMessenger(fForwardTo).SendMessage(msg);
				return B_SKIP_MESSAGE;
			}
		} break;

		default: {
		} break;
	}
	
	return result;
};

filter_result InputFilter::HandleKeys (BMessage *msg) {
	const char *keyStroke;
	int32 keyModifiers;
	
	msg->FindString("bytes", &keyStroke);
	msg->FindInt32("modifiers", &keyModifiers);

	switch (keyStroke[0]) {
		case B_RETURN: {
			if (fCommandSends == true) {
				if (keyModifiers & B_COMMAND_KEY) {
					BMessage *typing = new BMessage(IM::USER_STOPPED_TYPING);
					BMessenger(fParent->Window()).SendMessage(typing);			
				
					BMessenger(fParent->Window()).SendMessage(fMessage);
					fLastTyping = 0;
					return B_SKIP_MESSAGE;
				};
			} else {
				if ((keyModifiers & B_COMMAND_KEY) != B_COMMAND_KEY) {
					BMessage *typing = new BMessage(IM::USER_STOPPED_TYPING);
					BMessenger(fParent->Window()).SendMessage(typing);			
				
					BMessenger(fParent->Window()).SendMessage(fMessage);
					fLastTyping = 0;
					return B_SKIP_MESSAGE;
				} else {
					fParent->Insert(fParent->TextLength(), "\n", strlen("\n"));
				};
			};
		} break;
	
		case B_ESCAPE: {
		} break;
		
		default: {
			if ((system_time() - fLastTyping) > fInterval) {
				BMessenger(fParent->Window()).SendMessage(new BMessage(IM::USER_STARTED_TYPING));
				fLastTyping = system_time();
			}
//			filter_result result (B_DISPATCH_MESSAGE);
//			return result;
		} break;
	};

	return B_DISPATCH_MESSAGE;

};
