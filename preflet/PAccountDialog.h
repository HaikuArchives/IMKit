/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PACCOUNT_DIALOG_H
#define PACCOUNT_DIALOG_H

#include <app/Message.h>
#include <interface/Window.h>

class BBox;
class BMessenger;
class BPath;
class BTextControl;

class Divider;

class PAccountDialog : public BWindow {
	public:
						PAccountDialog(const char *title, const char *protocol, const char *account, BMessage settingsTemplate, BMessage settings, BMessenger *target, BMessage save, BMessage cancel);

		// BWindow Hooks
		virtual void	MessageReceived(BMessage* msg);
		virtual bool	QuitRequested(void);

		// Public
		const char		*AccountName();

	private:
		void			SendNotification(bool saved);
		
		BString			fOriginalAccount;
		BMessage		fTemplate;
		BMessage		fSettings;
	
		BBox			*fBox;
		BTextControl		*fAccountName;
		Divider			*fAccountNameDivider;
		BView			*fProtocolControl;
		
		BMessenger		*fTarget;
		BMessage		fSave;
		BMessage		fCancel;
};

#endif // PACCOUNT_DIALOG_H
