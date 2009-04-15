#ifndef AWAYMESSAGEWINDOW_H
#define AWAYMESSAGEWINDOW_H

#include <interface/Window.h>
#include <support/String.h>

class AccountInfo;
class BButton;
class BScrollView;
class BTextView;
class BView;

class AwayMessageWindow : public BWindow {
	public:
								AwayMessageWindow(AccountInfo *info = NULL);
			virtual				~AwayMessageWindow(void);
			
			// BWindow Hooks
			virtual bool		QuitRequested(void);
			virtual void 		MessageReceived(BMessage *msg);
		
	private:	
			BView				*fView;
			BTextView			*fTextView;
			BScrollView			*fScroller;
			float				fFontHeight;
			BButton				*fOkay;
			BButton				*fCancel;
			BString				fAccountInstance;
};

#endif // AWAYMESSAGEWINDOW_H
