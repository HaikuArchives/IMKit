/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef SETTINGS_INFO_H
#define SETTINGS_INFO_H

#include <app/Message.h>
#include <storage/Path.h>
#include <support/String.h>

class BView;
class SettingsController;

typedef enum {
	Server,
	Client,
	Protocol,
} settings_type;

class SettingsInfo {
	public:
							SettingsInfo(settings_type type, BPath path, BString name);

		// Public
		settings_type		Type(void) const;
		BPath				Path(void) const;
		const char			*Name(void) const;
		
		BView				*View(void) const;
		void				View(BView *view);
		SettingsController	*Controller(void) const;
		void				Controller(SettingsController *controller);
		BMessage			Template(void) const;
		void				Template(BMessage tmplate);
		BMessage			Settings(void) const;
		void				Settings(BMessage settings);						
		
	private:
		settings_type		fType;
		BPath				fPath;
		BString				fName;
		
		BView				*fView;
		SettingsController	*fController;
		BMessage			fTemplate;
		BMessage			fSettings;
};

#endif // SETTINGS_INFO_H
