/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PEOPLE_COLUMN_LIST_VIEW_H
#define _PEOPLE_COLUMN_LIST_VIEW_H

#include <map>

#ifdef __HAIKU__
#	include <common/columnlistview/haiku/ColumnListView.h>
#	include <common/columnlistview/haiku/ColumnTypes.h>
#else
#	include <common/columnlistview/zeta/ColumnListView.h>
#	include <common/columnlistview/zeta/ColumnTypes.h>
#endif
#include <common/ObjectList.h>

class BQuery;

typedef struct {
	BRow*				row;
	BObjectList<BQuery>	queries;
} ProtocolItem;

typedef std::map<const char*, ProtocolItem*> ProtocolItems;

class PeopleColumnListView : public BColumnListView {
public:
							PeopleColumnListView(const char* name);

	virtual	void			MessageReceived(BMessage* msg);

			void			Populate();

private:
			ProtocolItems	fProtocolItems;
};

#endif	// _PEOPLE_COLUMN_LIST_VIEW_H
