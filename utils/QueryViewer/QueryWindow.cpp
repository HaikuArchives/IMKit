#include "QueryWindow.h"

const float kEdgeSpacer = 5.0;
const char *kQueryDir = "/boot/home/config/settings/BeClan/QueryViewer/Queries";
const char *kCollapseAttr = "queryviewer:collapse";

QueryWindow::QueryWindow(BRect rect)
	: BWindow(rect, "QueryViewer", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS) {
	
	fView = new BView(Bounds(), "MainView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	AddChild(fView);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fView->SetHighColor(0, 0, 0, 0);
#endif

	BRect outline = Bounds();
	outline.InsetBy(kEdgeSpacer, kEdgeSpacer);
	outline.right = outline.right * 0.4;
	
	fQueryList = new BOutlineListView(outline, "Queries", B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fQueryList->SetSelectionMessage(new BMessage(qwQuerySelected));
	fQueryList->SetInvocationMessage(new BMessage(qwQueryInvoked));
		
	BScrollView *scroll = new BScrollView("Scroller", fQueryList,
		B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, 0, false, true);
	fView->AddChild(scroll);
	
	fListRect = Bounds();
	fListRect.InsetBy(kEdgeSpacer, kEdgeSpacer);
	fListRect.left = outline.right + kEdgeSpacer + B_V_SCROLL_BAR_WIDTH + kEdgeSpacer;
	
	entry_ref ref;
	BEntry entry(kQueryDir);
	if (entry.Exists() == false) {
		if (create_directory(kQueryDir, 0777) != B_OK) {
			BString msg = "Could not create the root query directory (";
			msg << kQueryDir << "). Please ensure this exists and try again.";
	
			BAlert *alert = new BAlert("Error", msg.String(), "Fish merchant!", NULL, NULL,
				B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
			alert->Go();
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
		};
	};

	if (get_ref_for_path(kQueryDir, &ref) != B_OK) {
		BString msg = "Could not get the query directory (";
		msg << kQueryDir << "). Please ensure this exists and try again.";
			 
		BAlert *alert = new BAlert("Error", msg.String(), "John West!", NULL, NULL,
			B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_STOP_ALERT);
		alert->Go();
		
		be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	};
	
	fRootItem = new IconCountItem("Queries", "", ReadNodeIcon(kQueryDir));
	fQueryList->AddItem(fRootItem);
	fDirectoryItem[ref] = fRootItem;
	CreateGroups(BDirectory(kQueryDir), fRootItem, fListRect);
	
	Show();

	fQueryList->Select(0);
	BMessage msg(qwQuerySelected);
	msg.AddInt32("index", 0);
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	BMessenger(this).SendMessage(msg);
#else
	BMessenger(this).SendMessage(&msg);
#endif

	fCurrentQView = NULL;
};

QueryWindow::~QueryWindow(void) {
	if (fQueryList) {
		int32 items = fQueryList->FullListCountItems();

		for (int32 i = 0; i < items; i++) {
			IconCountItem *item = ((IconCountItem *)fQueryList->ItemAt(i));
			if ((item) && (fQueryList->CountItemsUnder(item, false) > 0)) {
				bool collapse = !item->IsExpanded();
				WriteAttribute(BNode(item->Path()), kCollapseAttr,
					(char *)&collapse, sizeof(bool), B_STRING_TYPE);
			};
		};
	};
	
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);
};

//#pragma mark -

void QueryWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case qwQuerySelected: {
			int32 index = -1;
			if (msg->FindInt32("index", &index) != B_OK) return;
			
			IconCountItem *item = reinterpret_cast<IconCountItem *>(fQueryList->ItemAt(index));
			if (item == NULL) return;
			item->IsNew(false);
			fQueryList->Invalidate();
			
			if (fQueryList->CountItemsUnder(item, false) == 0) {
				cl_map::iterator cIt = fQueryViews.find(item->Path());
				if (cIt == fQueryViews.end()) return;

				if (fCurrentQView) fCurrentQView->Hide();
				fCurrentQView = cIt->second;
				fCurrentQView->Show();
			};
			
		} break;
		
		case qwQueryInvoked: {
			int32 index = -1;
			if (msg->FindInt32("index", &index) != B_OK) return;
			
			IconCountItem *item = reinterpret_cast<IconCountItem *>(fQueryList->ItemAt(index));
			if (item == NULL) return;
			
			entry_ref ref;
			if (get_ref_for_path(item->Path(), &ref) == B_OK) {
				BMessage open(B_REFS_RECEIVED);
				open.AddRef("refs", &ref);
				
				be_roster->Launch("application/x-vnd.Be-TRAK", &open);
			};
		} break;
		
		case qwQueryUpdated: {
			int32 count = -1;
			IconCountItem *item = NULL;

			if (msg->FindInt32("qclvCount", &count) != B_OK) return;
			if (msg->FindPointer("itemPointer", reinterpret_cast<void **>(&item)) != B_OK) return;

			item = reinterpret_cast<IconCountItem *>(item);
			
			item->Count(count);
			item->IsNew(true);
			BListItem *i = item;
			BListItem *super = NULL;
			
			while ((super = fQueryList->Superitem(i)) != NULL) {
				((IconCountItem *)super)->IsNew(true);
				i = super;
			};
			
			CountItemEntries(fRootItem);
			
			fQueryList->Invalidate();
		} break;
		
		case B_NODE_MONITOR: {
			msg->PrintToStream();
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) != B_OK) return;
			
			switch (opcode) {
				case B_ENTRY_CREATED: {				
					entry_ref ref;
					const char *name;
					msg->FindInt32("device", &ref.device);
					msg->FindInt64("directory", &ref.directory);
					msg->FindString("name", &name);
					ref.set_name(name);

					BEntry entry(&ref);
					BPath path(&ref);
					struct stat s;
					IconCountItem *item = new IconCountItem(path.Leaf(), path.Path(), 
						ReadNodeIcon(path.Path()));
					
					entry.GetStat(&s);

					node_ref parentNRef;
					BDirectory parent;
					BEntry parentEntry;
					entry_ref parentRef;
					
					parentNRef.device = ref.device;
					parentNRef.node = ref.directory;
					parent.SetTo(&parentNRef);
					parent.GetEntry(&parentEntry);
					parentEntry.GetRef(&parentRef);
					
					di_map::iterator dIt = fDirectoryItem.find(parentRef);
					if (dIt == fDirectoryItem.end()) return;

					
					if (S_ISDIR(s.st_mode)) {
						fQueryList->AddUnder(item, dIt->second);
						fDirectoryItem[parentRef] = item;
						
						CreateGroups(BDirectory(&ref), item, fListRect);
					} else {
					};

					
				} break;
			};
			
		} break;
		
		default: {
			BWindow::MessageReceived(msg);
		} break;
	};
};

//#pragma mark -

void QueryWindow::CreateGroups(BDirectory dir, BListItem *under, BRect rect) {
	if (dir.InitCheck() != B_OK) debugger(strerror(dir.InitCheck()));
	BEntry entry;

	node_ref nref;
	dir.GetNodeRef(&nref);
	watch_node(&nref, B_WATCH_ALL, BMessenger(this));

	for (int32 i = 0; dir.GetNextEntry(&entry, true) == B_OK; i++) {
		struct stat s;
		entry.GetStat(&s);
		BPath path;
		entry.GetPath(&path);
		BBitmap *icon = ReadNodeIcon(path.Path());
		IconCountItem *item = new IconCountItem(path.Leaf(), path.Path(), icon);
		
		if (S_ISDIR(s.st_mode)) {
			entry_ref ref;
			entry.GetRef(&ref);

			fQueryList->AddUnder(item, under);

			int32 length = -1;
			char *collapse = ReadAttribute(BNode(&entry), kCollapseAttr, &length);
			if ((length > 0) && (collapse[0] != 0x00)) fQueryList->Collapse(item);
			free(collapse);

			fDirectoryItem[ref] = item;
			CreateGroups(BDirectory(&entry), item, rect);
			
		} else if (S_ISREG(s.st_mode)) {
			BNode node(&entry);
			int32 length = -1;
			char *type = ReadAttribute(node, "BEOS:TYPE", &length);
			if ((length > 0) && (strncmp(type, kTrackerQueryType, length) == 0)) {
				entry_ref ref;
				entry.GetRef(&ref);
				
				BMessage *updateMsg = new BMessage(qwQueryUpdated);
				updateMsg->AddPointer("itemPointer", item);
				
				QueryColumnListView *view = new QueryColumnListView(rect,
					path.Path(), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED, ref,
					updateMsg, new BMessenger(this), 
					B_FANCY_BORDER);
				fQueryViews[path.Path()] = view;
				fView->AddChild(view);
				view->Hide();
				
				fQueryList->AddUnder(item, under);
			};
			free(type);
		};
	};
};

int32 QueryWindow::CountItemEntries(IconCountItem *item) {
	int32 ret = 0;
	int32 children = fQueryList->CountItemsUnder(item, true);
	
	if (children > 0) {
		for (int32 i = 0; i < children; i++) {
			BListItem *child = fQueryList->ItemUnderAt(item, true, i);
			if (child) ret += CountItemEntries((IconCountItem *)child);
		};
		
		item->Count(ret);
	} else {
		ret = item->Count();
	};
	
	return ret;
};

