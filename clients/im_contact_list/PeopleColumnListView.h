/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PEOPLE_COLUMN_LIST_VIEW_H
#define _PEOPLE_COLUMN_LIST_VIEW_H

#include <common/ColumnListView.h>
#include <common/ColumnTypes.h>

class PeopleColumnListView : public BColumnListView {
public:
						PeopleColumnListView(const char* name);

			void		Populate();
};

#endif	// _PEOPLE_COLUMN_LIST_VIEW_H
