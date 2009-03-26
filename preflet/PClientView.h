/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PCLIENTVIEW_H
#define PCLIENTVIEW_H

#include <interface/View.h>

#include "SettingsController.h"

class SettingsHost;

class PClientView : public BView, public SettingsController {
	public:
					PClientView(BRect frame, const char *name, const char *title);

		// SettingsController Hooks
		virtual status_t	Init(SettingsHost *host);
		virtual status_t	Save(BView *view, const BMessage *tmplate, BMessage *settings);
		virtual status_t	Revert(BView *view, const BMessage *tmplate);

	private:
		SettingsHost		*fHost;
};

#endif // PCLIENTVIEW_H
