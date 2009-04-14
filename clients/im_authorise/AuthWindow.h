#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <Window.h>
#include <libim/Contact.h>

class BBox;
class BButton;
class BScrollView;
class BTextView;
class BView;
class MultiLineStringView;

class AuthWindow : public BWindow {
	public:
							AuthWindow(entry_ref contact, const char *reason);
							~AuthWindow(void);

		// BWindow hooks
		void 				MessageReceived(BMessage *msg);
		bool 				QuitRequested(void);

	private:
		const char			*TranslateString(const char *str, ...);
	
		BView				*fView;
		BBox				*fBox;
		MultiLineStringView	*fInfoText;
		BTextView			*fAdditional;
		BScrollView			*fAdditionalScroll;
		
		BButton				*fAuthorise;
		BButton				*fDeny;

		IM::Contact			fContact;
};

#endif
