#include "MenuColumns.h"

#include <common/interface/IconMenuItem.h>

#include <stdio.h>

BPopUpMenu *generate_menu(BRow *row, BColumnListView *parent) {
	BStringField *pathField = reinterpret_cast<BStringField *>(row->GetField(kPathIndex));
	BStringField *nameField = reinterpret_cast<BStringField *>(row->GetField(kNameIndex));
	BPopUpMenu *menu = NULL;
	QueryColumnListView *qclv = reinterpret_cast<QueryColumnListView *>(parent);
	entry_ref ref;
	
	if (pathField == NULL) return menu;
	if (nameField == NULL) return menu;
	BPath refPath = pathField->String();
	refPath.Append(nameField->String());
	
	if (get_ref_for_path(refPath.Path(), &ref) != B_OK) return menu;

	char *type = qclv->MIMETypeFor(&ref);
	
	const char *dirs[2];
	dirs[0] = "/boot/home/config/settings/BeClan/QueryViewer/Actions/Generic/";

	BString typepath = type;
	typepath.ReplaceAll("/", "_");
	typepath.Prepend("/boot/home/config/settings/BeClan/QueryViewer/Actions/");
	dirs[1] = typepath.String();
	
	BString qpath = "/boot/home/config/settings/BeClan/QueryViewer/Actions/";
	qpath << qclv->Name();
	dirs[2] = qpath.String();
	
	menu = new BPopUpMenu("QCLVMenu");
	menu->SetFont(be_plain_font);
		
	for (int32 i = 2; i > -1; i--) {
		BDirectory dir(dirs[i]);
		entry_ref actionRef;
		bool added = false;
	
		while (dir.GetNextRef(&actionRef) == B_OK) {	
			BPath actionPath(&actionRef);
			BBitmap *icon = NULL;
	
			if (strcmp(actionRef.name, "_DEFAULT_") == 0) continue;
			if ((added == false) && (menu->CountItems() > 0))  menu->AddSeparatorItem();
	
			icon = ReadNodeIcon(actionPath.Path());
			
			BMessage *send = new BMessage(mscActionTaken);
			send->AddRef("targetRef", &ref);
			send->AddRef("actionRef", &actionRef);
			
			menu->AddItem(new IconMenuItem(icon, actionPath.Leaf(), NULL, send));
			added = true;
		};

	};
	
	if (menu->CountItems() > 0) {
		menu->SetTargetForItems(parent);
	} else {
		delete menu;
		menu = NULL;
	};
	return menu;
};

MenuStringColumn::MenuStringColumn(const char *title, float width, float maxWidth,
	float minWidth, uint32 truncate, alignment align)
	: BStringColumn(title, width, maxWidth, minWidth, truncate, align) {

	fMenu = NULL;
};

MenuStringColumn::~MenuStringColumn(void) {
	delete fMenu;
};

void MenuStringColumn::MouseDown(BColumnListView *parent, BRow *row, BField *field,
	BRect field_rect, BPoint point, uint32 buttons) {

	BMessage *msg = parent->Window()->CurrentMessage();
	msg->FindInt32("buttons", (int32 *)&buttons);
	
	switch (buttons) {
		case B_SECONDARY_MOUSE_BUTTON: {
			BPoint p2 = parent->ScrollView()->ConvertToScreen(point);
			p2.x -= 5.0;
			p2.y -= 5.0;

			delete fMenu;
			fMenu = generate_menu(row, parent);
			if (fMenu) fMenu->Go(p2, true, true, true);
		} break;
	};	
};

//#pragma mark -

MenuIntegerColumn::MenuIntegerColumn(const char *title, float width, float maxWidth,
	float minWidth, alignment align)
	: BIntegerColumn(title, width, maxWidth, minWidth, align) {

	fMenu = NULL;
};

MenuIntegerColumn::~MenuIntegerColumn(void) {
	delete fMenu;
};

void MenuIntegerColumn::MouseDown(BColumnListView *parent, BRow *row, BField *field,
	BRect field_rect, BPoint point, uint32 buttons) {

	BMessage *msg = parent->Window()->CurrentMessage();
	msg->FindInt32("buttons", (int32 *)&buttons);
	
	switch (buttons) {
		case B_SECONDARY_MOUSE_BUTTON: {
			BPoint p2 = parent->ScrollView()->ConvertToScreen(point);
			p2.x -= 5.0;
			p2.y -= 5.0;

			delete fMenu;
			fMenu = generate_menu(row, parent);
			if (fMenu) fMenu->Go(p2, true, true, true);
		} break;
	};	
};

//#pragma mark -

MenuSizeColumn::MenuSizeColumn(const char *title, float width, float maxWidth,
	float minWidth, alignment align)
	: BSizeColumn(title, width, maxWidth, minWidth, align) {

	fMenu = NULL;
};

MenuSizeColumn::~MenuSizeColumn(void) {
	delete fMenu;
};

void MenuSizeColumn::MouseDown(BColumnListView *parent, BRow *row, BField *field,
	BRect field_rect, BPoint point, uint32 buttons) {

	BMessage *msg = parent->Window()->CurrentMessage();
	msg->FindInt32("buttons", (int32 *)&buttons);
	
	switch (buttons) {
		case B_SECONDARY_MOUSE_BUTTON: {
			BPoint p2 = parent->ScrollView()->ConvertToScreen(point);
			p2.x -= 5.0;
			p2.y -= 5.0;

			delete fMenu;
			fMenu = generate_menu(row, parent);
			if (fMenu) fMenu->Go(p2, true, true, true);
		} break;
	};	
};

//#pragma mark -

MenuDateColumn::MenuDateColumn(const char *title, float width, float maxWidth,
	float minWidth, alignment align)
	: BDateColumn(title, width, maxWidth, minWidth, align) {

	fMenu = NULL;
};

MenuDateColumn::~MenuDateColumn(void) {
	delete fMenu;
};

void MenuDateColumn::MouseDown(BColumnListView *parent, BRow *row, BField *field,
	BRect field_rect, BPoint point, uint32 buttons) {

	BMessage *msg = parent->Window()->CurrentMessage();
	msg->FindInt32("buttons", (int32 *)&buttons);
	
	switch (buttons) {
		case B_SECONDARY_MOUSE_BUTTON: {
			BPoint p2 = parent->ScrollView()->ConvertToScreen(point);
			p2.x -= 5.0;
			p2.y -= 5.0;

			delete fMenu;
			fMenu = generate_menu(row, parent);
			if (fMenu) fMenu->Go(p2, true, true, true);
		} break;
	};	
};

//#pragma mark -

MenuBitmapColumn::MenuBitmapColumn(const char *title, float width, float maxWidth,
	float minWidth, alignment align)
	: BBitmapColumn(title, width, maxWidth, minWidth, align) {

	fMenu = NULL;
};

MenuBitmapColumn::~MenuBitmapColumn(void) {
	delete fMenu;
};

void MenuBitmapColumn::MouseDown(BColumnListView *parent, BRow *row, BField *field,
	BRect field_rect, BPoint point, uint32 buttons) {

	BMessage *msg = parent->Window()->CurrentMessage();
	msg->FindInt32("buttons", (int32 *)&buttons);
	
	switch (buttons) {
		case B_SECONDARY_MOUSE_BUTTON: {
			BPoint p2 = parent->ScrollView()->ConvertToScreen(point);
			p2.x -= 5.0;
			p2.y -= 5.0;

			delete fMenu;
			fMenu = generate_menu(row, parent);
			if (fMenu) fMenu->Go(p2, true, true, true);
		} break;
	};	
};
