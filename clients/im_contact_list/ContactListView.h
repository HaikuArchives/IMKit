/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_LIST_VIEW_H
#define _CONTACT_LIST_VIEW_H

#include <View.h>

class PeopleColumnListView;
class StatusView;

class ContactListView : public BView {
public:
									ContactListView(const char* name);

	// BView hooks
	virtual	BSize					MinSize();

private:
			PeopleColumnListView*	fListView;
			StatusView*				fStatusView;
};

#endif	// _CONTACT_LIST_VIEW_H
