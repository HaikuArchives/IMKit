/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef SETTINGS_CONTROLLER_H
#define SETTINGS_CONTROLLER_H

#include <app/Message.h>
#include <interface/View.h>

class SettingsHost;

class SettingsController {
	public:
	
		// Hooks
		virtual status_t	Init(SettingsHost *host);
		virtual status_t	Save(BView *view, const BMessage *tmplate, BMessage *settings);
		virtual status_t	Revert(BView *view, const BMessage *tmplate);
		
	private:
};

#endif // SETTINGS_CONTROLLER_H
