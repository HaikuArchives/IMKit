#ifndef MENUCOLUMNS_H
#define MENUCOLUMNS_H

#include <Directory.h>
#include <Entry.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Window.h>

#include "CLV/ColumnListView.h"
#include "CLV/ColumnTypes.h"
#include "QueryColumnListView.h"

#include "IMKitUtilities.h"

extern const int32 kPathIndex;
extern const int32 kNameIndex;

enum {
	mscActionTaken = 'ms01',
};

class MenuStringColumn : public BStringColumn {
	public:
							MenuStringColumn(const char *title, float width,
								float maxWidth, float minWidth, uint32 truncate,
								alignment align = B_ALIGN_LEFT);
							~MenuStringColumn(void);
							
		virtual void		MouseDown(BColumnListView *parent, BRow *row,
								BField *field, BRect field_rect, BPoint point,
								uint32 buttons);

	private:
			BPopUpMenu		*fMenu;
};

class MenuIntegerColumn : public BIntegerColumn {
	public:
							MenuIntegerColumn(const char *title, float width,
								float maxWidth, float minWidth,
								alignment align = B_ALIGN_LEFT);
							~MenuIntegerColumn(void);
							
		virtual void		MouseDown(BColumnListView *parent, BRow *row,
								BField *field, BRect field_rect, BPoint point,
								uint32 buttons);

	private:
			BPopUpMenu		*fMenu;
};

class MenuSizeColumn : public BSizeColumn {
	public:
							MenuSizeColumn(const char *title, float width,
								float maxWidth, float minWidth,
								alignment align = B_ALIGN_LEFT);
							~MenuSizeColumn(void);
							
		virtual void		MouseDown(BColumnListView *parent, BRow *row,
								BField *field, BRect field_rect, BPoint point,
								uint32 buttons);

	private:
			BPopUpMenu		*fMenu;
};

class MenuDateColumn : public BDateColumn {
	public:
							MenuDateColumn(const char *title, float width,
								float maxWidth, float minWidth,
								alignment align = B_ALIGN_LEFT);
							~MenuDateColumn(void);
							
		virtual void		MouseDown(BColumnListView *parent, BRow *row,
								BField *field, BRect field_rect, BPoint point,
								uint32 buttons);

	private:
			BPopUpMenu		*fMenu;
};

class MenuBitmapColumn : public BBitmapColumn {
	public:
							MenuBitmapColumn(const char *title, float width,
								float MaxWidth, float minWidth,
								alignment align = B_ALIGN_LEFT);
							~MenuBitmapColumn(void);

		virtual void		MouseDown(BColumnListView *parent, BRow *row,
								BField *field, BRect field_rect, BPoint point,
								uint32 buttons);

	private:
			BPopUpMenu		*fMenu;
};

#endif
