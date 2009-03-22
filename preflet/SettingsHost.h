/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef SETTINGS_HOST_H
#define SETTINGS_HOST_H

class SettingsController;

class SettingsHost {
	public:
		
		// Hooks
		virtual void		ControllerModified(SettingsController *controller) = 0;
		
	private:
};

#endif // SETTINGS_HOST_H
