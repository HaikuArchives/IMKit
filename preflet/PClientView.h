/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PCLIENTVIEW_H
#define PCLIENTVIEW_H

#include <interface/View.h>
#include <support/String.h>

#include "SettingsController.h"

class SettingsHost;

class PClientView : public BView, public SettingsController {
	public:
							PClientView(BRect frame, const char *name, const char *title, BMessage tmplate, BMessage settings);

		// BView Hooks
		virtual void		AttachedToWindow(void);
		virtual void		MessageReceived(BMessage *msg);

		// SettingsController Hooks
		virtual status_t	Init(SettingsHost *host);
		virtual status_t	Save(BView *view, const BMessage *tmplate, BMessage *settings);
		virtual status_t	Revert(BView *view, const BMessage *tmplate);

		// Public
		bool				ShowHeading(void) const;
		void				SetShowHeading(bool show);

	private:
		float				BuildGUI(void);

		BString				fTitle;
		BMessage			fTemplate;
		BMessage			fSettings;

		bool				fShowHeading;
	
		SettingsHost		*fHost;
};

#endif // PCLIENTVIEW_H
