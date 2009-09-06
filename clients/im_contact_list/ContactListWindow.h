/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTAC_LIST_WINDOW_H
#define _CONTAC_LIST_WINDOW_H

#include <Window.h>

#include <libim/Manager.h>

class ContactListView;

class ContactListWindow : public BWindow {
public:
										ContactListWindow();

				status_t				InitCheck();

		virtual	bool					QuitRequested();

private:
				IM::Manager*			fManager;
				ContactListView*		fView;
};

#endif	// _CONTAC_LIST_WINDOW_H
