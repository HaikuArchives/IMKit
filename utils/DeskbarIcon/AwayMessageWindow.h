#ifndef AWAYMESSAGEWINDOW_H
#define AWAYMESSAGEWINDOW_H

#include <View.h>
#include <TextView.h>
#include <Window.h>
#include <ScrollView.h>
#include <Button.h>

#include <stdlib.h>

#include <libim/Manager.h>
#include <libim/Constants.h>


class AwayMessageWindow : public BWindow {
	public:
								AwayMessageWindow(const char *protocol = NULL);
					virtual		~AwayMessageWindow();
			virtual bool		QuitRequested(void);
			virtual void 		MessageReceived(BMessage *);
		
	private:
		enum {
			CANCEL_AWAY,
			SET_AWAY
		};
		
			BView				*fView;
			BTextView			*fTextView;
			BScrollView			*fScroller;
			float				fFontHeight;
			BButton				*fOkay;
			BButton				*fCancel;
			char				*fProtocol;
};

#endif
