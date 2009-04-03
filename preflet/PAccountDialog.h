/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PACCOUNT_DIALOG_H
#define PACCOUNT_DIALOG_H

#include <app/Message.h>
#include <interface/Window.h>

class BBox;
class BButton;
class BMessenger;
class BPath;
class BTextControl;

class Divider;
class PClientView;

class PAccountDialog : public BWindow {
	public:
							PAccountDialog(const char *title, const char *protocol, const char *account, BMessage settingsTemplate, BMessage settings, BMessenger *target, BMessage save, BMessage cancel);

		// BWindow Hooks
		virtual void		MessageReceived(BMessage* msg);
		virtual bool		QuitRequested(void);

		// Public
		const char			*AccountName(void);

	private:
#ifndef __HAIKU__
		void				LayoutGUI(void);
#endif
		void				SendNotification(bool saved);
		
		BString				fOriginalAccount;
		BMessage			fTemplate;
		BMessage			fSettings;
	
		BTextControl		*fAccountName;
		Divider				*fAccountNameDivider;
		PClientView			*fProtocolControl;
		BButton				*fCancelButton;
		BButton				*fOKButton;

		BMessenger			*fTarget;
		BMessage			fSave;
		BMessage			fCancel;
};

#endif // PACCOUNT_DIALOG_H
