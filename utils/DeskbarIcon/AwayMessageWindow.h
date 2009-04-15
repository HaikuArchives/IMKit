#ifndef AWAYMESSAGEWINDOW_H
#define AWAYMESSAGEWINDOW_H

#include <Window.h>

class BButton;
class BScrollView;
class BTextView;
class BView;

class AwayMessageWindow : public BWindow {
	public:
								AwayMessageWindow(const char *protocol = NULL);
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
			char				*fProtocol;
};

#endif // AWAYMESSAGEWINDOW_H
