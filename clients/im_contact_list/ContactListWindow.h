/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTAC_LIST_WINDOW_H
#define _CONTAC_LIST_WINDOW_H

#include <interface/Window.h>

class PeopleColumnListView;
class StatusView;

class ContactListWindow : public BWindow {
public:
										ContactListWindow();

		virtual	bool					QuitRequested();

private:
				PeopleColumnListView*	fListView;
				StatusView*				fStatusView;
};

#endif	// _CONTAC_LIST_WINDOW_H
