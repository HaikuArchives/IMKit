#include "QueryColumnListView.h"

const char *kTrackerQueryVolume = "_trk/qryvol1";
const char *kTrackerQueryPredicate = "_trk/qrystr";
const char *kTrackerQueryInitMime = "_trk/qryinitmime";
const char *kTrackerQueryType = "application/x-vnd.Be-query";

const char *kAttrStatName ="_stat/name";
const char *kAttrStatSize = "_stat/size";
const char *kAttrStatModified = "_stat/modified";
const char *kAttrStatCreated = "_stat/created";
const char *kAttrStatMode = "_stat/mode";
const char *kAttrStatOwner = "_stat/owner";
const char *kAttrStatGroup = "_stat/group";
const char *kAttrPath = "_trk/path";
const char *kAttrMIMEType = "BEOS:TYPE";

const char *kFolderState = "_trk/columns_le";
const char *kViewState = "_trk/viewstate_le";
const float kSnoozePeriod = 1000 * 1000 * 0.5;
const int32 kIconSize = 16;
const int32 kPathIndex = 6;
const int32 kNameIndex = 1;

mime_map QueryColumnListView::fMimeTypes;

QueryColumnListView::QueryColumnListView(BRect rect, const char *name,
	uint32 resizingMode, uint32 drawFlags, entry_ref ref, BMessage *msg,
	BMessenger *notify, border_style border,
	bool showHorizontalScrollbar)
	: BColumnListView(rect, name, resizingMode, drawFlags, border,
		showHorizontalScrollbar) {
		
	fMessage = msg;
	fNotify = notify;

	fRef = ref;
	
	char *buffer = NULL;
	int32 length;
	ExtractVolumes(&fRef, &fVolumes);
	ExtractPredicate(&ref, &buffer, &length);
	fPredicate = buffer;
	free(buffer);
	fInitialFetch = true;
};

QueryColumnListView::~QueryColumnListView(void) {
	if (fNotify) delete fNotify;
	if (fMessage) delete fMessage;
	
	public_info_map::iterator pIt;
	for (pIt = fPublicCols.begin(); pIt != fPublicCols.end(); pIt++) delete pIt->second;

	querylist::iterator qIt;
	for (qIt = fQueries.begin(); qIt != fQueries.end(); qIt++) delete (*qIt);
	
	stop_watching(this);
};

//#pragma mark -

void QueryColumnListView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case qclvAddRow: {
			entry_ref ref;
			if (msg->FindRef("ref", &ref) == B_OK) AddRowByRef(&ref);
		} break;
		
		case B_QUERY_UPDATE: {
			int32 opcode = 0;
			if (msg->FindInt32("opcode", &opcode) != B_OK) return;
			result r;
					
			switch (opcode) {
				case B_ENTRY_CREATED: {
					const char *name;
				
					msg->FindInt32("device", &r.ref.device); 
					msg->FindInt64("directory", &r.ref.directory); 
					msg->FindString("name", &name); 
					r.ref.set_name(name);

					msg->FindInt32("device", &r.nref.device);
					msg->FindInt64("node", &r.nref.node);
										
					fResults[r.ref] = r;
					fPending.push_back(r.ref);

					status_t watch = watch_node(&r.nref, B_WATCH_ALL, this);
					if (watch != B_OK) {
						BPath path(&r.ref);
						printf("Error starting node monitor for \"%s\": %s\n",
							path.Path(), strerror(watch));
					};
				} break;
				
				case B_ENTRY_REMOVED: {
					node_ref nref;
					resultmap::iterator rIt;

					msg->FindInt32("device", &nref.device);
					msg->FindInt64("node", &nref.node);

					for (rIt = fResults.begin(); rIt != fResults.end(); rIt++) {
						result r = rIt->second;
						
						if (nref == r.nref) {
							RemoveRowByRef(&r.ref);
							status_t watch = watch_node(&nref, B_STOP_WATCHING, this);
							if (watch != B_OK) {
								BPath path(&r.ref);
								printf("Error stopping node monitor for \"%s\": %s\n",
									path.Path(), strerror(watch));
							};
							
							fResults.erase(r.ref);
							break;
						};
					};
				} break;
			};
		
			if ((fNotify) && (fNotify->IsValid()) && (fMessage)) {
				BMessage send(*fMessage);
				send.AddInt32("qclvCount", fResults.size());

#if B_BEOS_VERSION > B_BEOS_VERSION_5
				fNotify->SendMessage(send);
#else
				fNotify->SendMessage(&send);
#endif
			};
		} break;
		
		case B_NODE_MONITOR: {
			int32 opcode = B_ERROR;
			resultmap::iterator rIt = fResults.begin();
			node_ref nref;

			if (msg->FindInt32("opcode", &opcode) != B_OK) return;
		
			switch (opcode) {
				case B_STAT_CHANGED:
				case B_ATTR_CHANGED: {
					if (msg->FindInt32("device", &nref.device) != B_OK) return;
					if (msg->FindInt64("node", &nref.node) != B_OK) return;					
				
					for (; rIt != fResults.end(); rIt++) {
						result r = rIt->second;

						if (nref == r.nref) {
							RemoveRowByRef(&r.ref);
							AddRowByRef(&r.ref);
							break;
						};
					};
				} break;
				
				case B_ENTRY_REMOVED: {
					if (msg->FindInt32("device", &nref.device) != B_OK) return;
					if (msg->FindInt64("node", &nref.node) != B_OK) return;					
				
					for (; rIt != fResults.end(); rIt++) {
						result r = rIt->second;
						
						if (nref == r.nref) {
							RemoveRowByRef(&r.ref);
							fResults.erase(r.ref);
							break;
						};
					};
				} break;
				
				case B_ENTRY_MOVED: {
					msg->PrintToStream();
				
					entry_ref ref;
					const char *name;
					if (msg->FindInt32("device", &nref.device) != B_OK) return;
					if (msg->FindInt64("from directory", &nref.node) != B_OK) return;
					if (msg->FindString("name", &name) != B_OK) return;

					ref.set_name(name);
					ref.directory = nref.node;
					ref.device = nref.device;

					RemoveRowByRef(&ref);
					fResults.erase(ref);
					
					if (msg->FindInt64("to directory", &nref.node) != B_OK) return;
					
					result r;
					r.ref.set_name(name);
					r.ref.device = nref.device;
					r.ref.directory = nref.node;
					r.nref = nref;
					
					fResults[r.ref] = r;
					AddRowByRef(&r.ref); 
				
				} break;
			};
		} break;
				
		case qclvRequestPending: {
			pending_stack::iterator pIt;
			BMessage reply(B_REPLY);
			for (pIt = fPending.begin(); pIt != fPending.end(); pIt++) {
				entry_ref& r = (*pIt);
				reply.AddRef("ref", &r);
			};
			fPending.clear();
			
			if (fInitialFetch) {
				reply.AddBool("initial", fInitialFetch);
				fInitialFetch = false;
			};
			
			msg->SendReply(&reply);
		} break;
		
		case qclvInvoke: {
			BRow *row = NULL;
			
			while ((row = CurrentSelection(row)) != NULL) {
				entry_ref ref;
				if (RefForRow(row, &ref) == B_OK) {
					entry_ref actionRef = ActionFor(&ref);
					BMessage open(B_REFS_RECEIVED);
					open.AddRef("refs", &ref);
		
					be_roster->Launch(&actionRef, &open);	
				};
			};
			
		} break;
		
		case mscActionTaken: {
			char *type = NULL;
			int32 length = 0;
			BMessage open(B_REFS_RECEIVED);

			entry_ref actionRef;
			entry_ref targetRef;
			msg->FindRef("actionRef", &actionRef);
			msg->FindRef("targetRef", &targetRef);

			type = ReadAttribute(BNode(&targetRef), "BEOS:TYPE", &length);
			type = (char *)realloc(type, (length + 1) * sizeof(char));
			type[length + 1] = '\0';
			
			BRow *selected = NULL;
			while ((selected = CurrentSelection(selected)) != NULL) {
				entry_ref currentRef;
				if (RefForRow(selected, &currentRef) == B_OK) {
					BNode currNode(&currentRef);
					char *currType = NULL;
					int32 currLength = -1;

					currType = ReadAttribute(currNode, "BEOS:TYPE", &currLength);
					currType = (char *)realloc(currType, (currLength + 1) * sizeof(char));
					currType[length + 1] = '\0';
					
					if (strcmp(type, currType) == 0) open.AddRef("refs", &currentRef);
					
					free(currType);
				};
			};
			
			free(type);
			
			be_roster->Launch(&actionRef, &open);
		} break;
		
		default: {
			BColumnListView::MessageReceived(msg);
		};
	};
};

void QueryColumnListView::KeyDown(const char *bytes, int32 numBytes) {
	char c = bytes[0];
	
	switch (c) {
		case B_DELETE: {
			BRow *row = NULL;
		
			while ((row = CurrentSelection()) != NULL) {
				if (row) {
					BStringField *pathField = reinterpret_cast<BStringField *>(row->GetField(kPathIndex));
					BStringField *nameField = reinterpret_cast<BStringField *>(row->GetField(kNameIndex));
					
					if ((pathField != NULL) && (nameField != NULL)) {
						BPath path = pathField->String();
						path.Append(nameField->String());
	
						entry_ref ref;
						if (get_ref_for_path(path.Path(), &ref) == B_OK) {
							RemoveRowByRef(&ref);
							unlink(path.Path());
						};
					};
				};
			};
		} break;

		case B_HOME: {
			if ((modifiers() & B_SHIFT_KEY) != 0) {
				int32 maxIndex = IndexOf(FocusRow());
				for (int32 i = 0; i < maxIndex; i++) {
					BRow *row = RowAt(i);
					if (row) AddToSelection(row);
				};
			};
			SetFocusRow(0L, (modifiers() & B_CONTROL_KEY) == 0);
			ScrollTo(FocusRow());
		} break;
		
		case B_END: {
			int32 startIndex = IndexOf(FocusRow());
			int32 maxIndex = CountRows();

			if ((modifiers() & B_SHIFT_KEY) != 0) {
				for (int32 i = startIndex; i < maxIndex; i++) {
					BRow *row = RowAt(i);
					if (row) AddToSelection(row);
				};
			};
			SetFocusRow(maxIndex - 1, (modifiers() & B_CONTROL_KEY) == 0);
			ScrollTo(FocusRow());
		} break;
				
		default: {
			BColumnListView::KeyDown(bytes, numBytes);
		};
	};
};

void QueryColumnListView::AttachedToWindow(void) {
	fMIMEString = MIMEType();

	vollist::iterator vIt;
	for (vIt = fVolumes.begin(); vIt != fVolumes.end(); vIt++) {
		BQuery *query = new BQuery();
		BVolume& v = (*vIt);
		
		query->SetPredicate(fPredicate.String());
		query->SetTarget(this);
		query->SetVolume(&v);

		query->Fetch();
		fQueries.push_back(query);
	};

	AddStatColumns();
	BMessage msg;
	BMimeType mime(fMIMEString.String());
	mime.GetAttrInfo(&msg);
	AddMIMEColumns(&msg);
	
	BPath path(&fRef);
	int32 length = -1;
	char *value = ReadAttribute(path.Path(), kFolderState, &length);
	if (length < 0) {
		BString queryPath = fMIMEString;
		queryPath.ReplaceAll("/", "_");
		queryPath.Prepend("/boot/home/config/settings/Tracker/DefaultQueryTemplates/");
		
		length = -1;
		value = ReadAttribute(queryPath.String(), kFolderState, &length);
	};

	if (length > 1) {		
		BMallocIO buffer;
		buffer.WriteAt(0, value, length);
		buffer.Seek(0, SEEK_SET);
		ExtractColumnState(&buffer);
	};
	free(value);
	
	value = ReadAttribute(path.Path(), "_trk/viewstate_le", &length);
	if (length < 0) {
		BString queryPath = fMIMEString;
		queryPath.ReplaceAll("/", "_");
		queryPath.Prepend("/boot/home/config/settings/Tracker/DefaultQueryTemplates/");
		
		length = -1;
		value = ReadAttribute(queryPath.String(), kViewState, &length);
	};
	
	if (length > 1) {
		BMallocIO buffer;
		buffer.WriteAt(0, value, length);
		buffer.Seek(0, SEEK_SET);
		ExtractViewState(&buffer);
	};
	free(value);

	fSelfMsgr = new BMessenger(this);
	
	BString name = fRef.name;
	name << " Background Adder";
	fThreadID = spawn_thread(BackgroundAdder, name.String(), B_LOW_PRIORITY, this);
	if (fThreadID > B_ERROR) resume_thread(fThreadID);

	SetTarget(this);
	SetInvocationMessage(new BMessage(qclvInvoke));
};

status_t QueryColumnListView::AddRowByRef(entry_ref *ref) {
	ref_map::iterator rIt = fRefRows.find(*ref);
	if (rIt != fRefRows.end()) return B_ERROR;
	
	ia_map::iterator iIt;
	BPath path(ref);
	BNode node(ref);
	BFile file(ref, B_READ_ONLY);
	off_t size;
	struct stat s;

	node.GetStat(&s);

	BRow *row = new BRow(20);

	file.GetSize(&size);
	int32 index = 0;
	
	BBitmap *icon = getBitmap(path.Path(), kIconSize);
	row->SetField(new BBitmapField(icon), index++);
	row->SetField(new BStringField(ref->name), index++);
	row->SetField(new BSizeField(size), index++);
	row->SetField(new BDateField(&s.st_mtime), index++);
	row->SetField(new BDateField(&s.st_ctime), index++);
	
	char *type = MIMETypeFor(ref);
	char kind[B_MIME_TYPE_LENGTH];
	BMimeType mimedb(type);
	mimedb.GetShortDescription(kind);
	free(type);
	
	row->SetField(new BStringField(kind), index++);
	
	BPath parentPath;
	path.GetParent(&parentPath);
	row->SetField(new BStringField(parentPath.Path()), index++);
	
	row->SetField(new BStringField(""), index++);		
	
	for (iIt = fIndexAttr.begin(); iIt != fIndexAttr.end(); iIt++) {
		attr_info info;
		info.size = 0;
		info.type = 0;
		node.GetAttrInfo(iIt->second.String(), &info);
		char *value = (char *)calloc(info.size, sizeof(char));
		int32 read = node.ReadAttr(iIt->second.String(), info.type, 0, value, info.size);
			
		BField *field = NULL;
		
		if (read < 0) {
			type_map::iterator aIt = fAttrTypes.find(iIt->second);
			if (aIt == fAttrTypes.end()) {
				printf("%s doesn't exist in our typemap\n", iIt->second.String());
				continue;
			};
			
			switch (aIt->second) {
				case B_CHAR_TYPE:
				case B_STRING_TYPE: {
					field = new BStringField("");
				} break;
				
				case B_INT8_TYPE:
				case B_INT16_TYPE:
				case B_INT32_TYPE:
				case B_INT64_TYPE: {
					field = new BIntegerField(-1);
				} break;
				
				case B_SIZE_T_TYPE: {
					field = new BSizeField(0);
				} break;
				
				case B_TIME_TYPE: {
					time_t time = 0;
					field = new BDateField(&time);
				} break;
				
				default: {
					printf("%s (%4.4s) was unhandled!\n", iIt->second.String(),
						&aIt->second);
				} break;
			};
			
		} else {
			switch (info.type) {
				case B_CHAR_TYPE: {
					char buffer[] = { value[0], '\0' };
					field = new BStringField(buffer);
				} break;
				case B_STRING_TYPE: {
					value = (char *)realloc(value, sizeof(char) * (read + 1));
					value[read] = '\0';
					field = new BStringField((char *)value);
				} break;
	
				case B_INT8_TYPE: {
					int8 *intValue = (int8 *)value;
					field = new BIntegerField(*intValue);
				} break;
				case B_INT16_TYPE: {
					int16 *intValue = (int16 *)value;
					field = new BIntegerField(*intValue);
				} break;
				case B_INT32_TYPE: {
					int32 *intValue = (int32 *)value;
					field = new BIntegerField(*intValue);
				} break;
				case B_INT64_TYPE: {
					int64 *intValue = (int64 *)value;
					field = new BIntegerField(*intValue);
				} break;
				
				case B_SIZE_T_TYPE: {
					size_t *sizeValue = (size_t *)value;
					field = new BSizeField(*sizeValue);
				} break;
				
				case B_TIME_TYPE: {
					time_t *timeValue = (time_t *)value;
					field = new BDateField(timeValue);
				} break;
				
				default: {
					printf("%s (%4.4s) was unhandled!\n", iIt->second.String(), &info.type);
				};
			};
		};

		if (field != NULL) row->SetField(field, iIt->first);
		free(value);
	};


	AddRow(row);
	fRefRows[*ref] = row;

	return B_OK;
};

status_t QueryColumnListView::RemoveRowByRef(entry_ref *ref) {
	ref_map::iterator rIt = fRefRows.find(*ref);
	status_t ret = B_ERROR;
	if (rIt != fRefRows.end()) {
		RemoveRow(rIt->second);
		fRefRows.erase(rIt);
		ret = B_OK;
	};
	
	return B_OK;
};

status_t QueryColumnListView::RefForRow(BRow *row, entry_ref *ref) {
	BStringField *pathField = reinterpret_cast<BStringField *>(row->GetField(kPathIndex));
	BStringField *nameField = reinterpret_cast<BStringField *>(row->GetField(kNameIndex));
	status_t status = B_ERROR;
	
	if ((pathField != NULL) && (nameField != NULL)) {
		BPath path = pathField->String();
		path.Append(nameField->String());

		status = get_ref_for_path(path.Path(), ref);
	};
	
	return status;
};

const char *QueryColumnListView::MIMEType(void) {
	if (fMimeTypes.size() == 0) {
		BMimeType mimeDB;
		BMessage types;
		mimeDB.GetInstalledTypes(&types);
			
		const char *type;
		for (int32 i = 0; types.FindString("types", i, &type) == B_OK; i++) {
			char name[B_MIME_TYPE_LENGTH];
			memset(name, '\0', B_MIME_TYPE_LENGTH);
			BMimeType mime(type);
			if (mime.GetShortDescription(name) == B_OK) fMimeTypes[name] = type;
		};
	};

	const char *ret = NULL;
	char *initial = ReadAttribute(BNode(&fRef), kTrackerQueryInitMime);
	mime_map::iterator qIt = fMimeTypes.find(initial);
	if (qIt != fMimeTypes.end()) ret = qIt->second.String();
	
	return ret;
};

char *QueryColumnListView::MIMETypeFor(entry_ref *ref) {
	int32 length = -1;
	char *type = ReadAttribute(BNode(ref), "BEOS:TYPE", &length);
	if ((type) && (length > 0)) {
		type = (char *)realloc(type, (length + 1) * sizeof(char));
		type[length] = '\0';
	};
	
	return type;
};

const char *QueryColumnListView::Name(void) {
	return fRef.name;
};

//#pragma mark -

status_t QueryColumnListView::ExtractColumnState(BMallocIO *source) {
	status_t ret = B_ERROR;

//	Data is;
//	int32, int32; Key / Version
//	int32 nameLen, char [nameLen + 1]
//	float offset, float width
//	align alignment
//	int32 internalLen, char [internalLen + 1]
//	uint32 hash
//	uint32 type
//	bool statField
//	bool editable

//	Need to offset 1 for the icon col		
	for (int32 i = 1; i < CountColumns(); i++) SetColumnVisible(i, false);
	
	while (source->Position() < source->BufferLength()) {
		char *publicName = NULL;
		char *internalName = NULL;
		float width = -1;
		alignment align = B_ALIGN_LEFT;
		uint32 type = 0;
		int32 length = -1;
		uint32 hash = 0;
				
		source->Seek(sizeof(int32), SEEK_CUR); // Key
		source->Seek(sizeof(int32), SEEK_CUR); // Version
		
		source->Read(&length, sizeof(int32));
		publicName = (char *)calloc(length + 1, sizeof(char));
		source->Read(publicName, length + 1);
			
		source->Seek(sizeof(float), SEEK_CUR); // Offset
		source->Read(&width, sizeof(float));
		source->Read(&align, sizeof(alignment));
		
		source->Read(&length, sizeof(int32));
		internalName = (char *)calloc(length + 1, sizeof(char));
		source->Read(internalName, length + 1);
		
		source->Read(&hash, sizeof(uint32));
		
		public_info_map::iterator pIt = fPublicCols.find(publicName);
		if (pIt != fPublicCols.end()) {
			hash = B_SWAP_INT32(hash);
			pIt->second->col->SetWidth(width);
			pIt->second->hash = hash;
			fHashCols[hash] = pIt->second;
		} else {
			printf("Couldn't find a place to store the hash for %s/%s\n",
				internalName, publicName);
		};
		
		source->Read(&type, sizeof(int32));

		bool isStat = false;
		source->Read(&isStat, sizeof(bool));
		source->Seek(sizeof(bool), SEEK_CUR); // editable
	
		ai_map::iterator iIt = fAttrIndex.find(internalName);
		if (iIt != fAttrIndex.end()) SetColumnVisible(iIt->second, true);
		
		free(publicName);
		free(internalName);
	};

	return ret;
};

status_t QueryColumnListView::ExtractViewState(BMallocIO *buffer) {
//	Data is;
//	uint32: viewmode
//	uint32: last icon mode
//	BPoint: list origin
//	BPoint: icon origin
//	uint32: Primary sort hash
//	uint32: Primary sort type
//	uint32: Secondary sort hash
//	uint32: Secondary sort type
//	bool: Reverse sort

	uint32 primaryAttr = 0;
	uint32 primaryType = 0;
	uint32 secondaryAttr = 0;
	uint32 secondaryType = 0;
	bool reverse = false;

	buffer->Seek(sizeof(uint32), SEEK_CUR);	// Key
	buffer->Seek(sizeof(int32), SEEK_CUR);	// Version
	buffer->Seek(sizeof(uint32), SEEK_CUR);	// View mode
	buffer->Seek(sizeof(uint32), SEEK_CUR);	// Last icon mode
	buffer->Seek(sizeof(BPoint), SEEK_CUR);	// List origin
	buffer->Seek(sizeof(BPoint), SEEK_CUR);	// Icon origon

	buffer->Read(&primaryAttr, sizeof(primaryAttr));
	buffer->Read(&primaryType, sizeof(primaryType));		
	primaryAttr = B_SWAP_INT32(primaryAttr);
	primaryType = B_SWAP_INT32(primaryType);
	
	buffer->Read(&secondaryAttr, sizeof(secondaryAttr));
	buffer->Read(&secondaryType, sizeof(secondaryType));
	secondaryAttr = B_SWAP_INT32(secondaryAttr);
	secondaryType = B_SWAP_INT32(secondaryType);
	
	buffer->Read(&reverse, sizeof(reverse));
	
	bool addToSort = false;
	hash_info_map::iterator hIt = fHashCols.find(primaryAttr);
	if (hIt != fHashCols.end()) {
		if (hIt->second->type == primaryType) {
			SetSortColumn(hIt->second->col, addToSort, reverse);
			addToSort = true;
		};
	};
		
	hIt = fHashCols.find(secondaryAttr);
	if (hIt != fHashCols.end()) {
		if (hIt->second->type == secondaryType) {
			SetSortColumn(hIt->second->col, addToSort, reverse);
		};
	};
};

void QueryColumnListView::AddColInfo(col_info *column) {
	fInternalCols[column->internalName] = column;
	fPublicCols[column->publicName] = column;
	fHashCols[column->hash] = column;
};

col_info *QueryColumnListView::MakeColInfo(BColumn *col, uint32 hash,
	const char *publicName, const char *internalName, uint32 type) {

	col_info *ret = new col_info;
	ret->col = col;
	ret->hash = hash;
	ret->publicName.SetTo(publicName);
	ret->internalName.SetTo(internalName);
	ret->type = type;
	
	return ret;
};

status_t QueryColumnListView::AddStatColumns(void) {
	int32 index = 0;
	col_info *colInfo = NULL;

	MenuBitmapColumn *icon = new MenuBitmapColumn("", 20, 20, 20, B_ALIGN_CENTER);
	icon->SetShowHeading(false);
	AddColumn(icon, index++);

	MenuStringColumn *name = new MenuStringColumn("Name", 200, 100, 300,
		B_TRUNCATE_END, B_ALIGN_LEFT);
	AddColumn(name, index++);
	colInfo = MakeColInfo(name, CalculateHash("Name", B_STRING_TYPE), "Name",
		kAttrStatName, B_STRING_TYPE);
	AddColInfo(colInfo);

	MenuSizeColumn *size = new MenuSizeColumn("Size", 50, 25, 100, B_ALIGN_RIGHT);
	AddColumn(size, index++);
	colInfo = MakeColInfo(size, CalculateHash("Size", B_OFF_T_TYPE), "Size",
		kAttrStatSize, 	B_OFF_T_TYPE);
	AddColInfo(colInfo);

	MenuDateColumn *modified = new MenuDateColumn("Modified", 200, 100, 300,
		B_ALIGN_LEFT);
	AddColumn(modified, index++);
	colInfo = MakeColInfo(modified, CalculateHash("Modified", B_TIME_TYPE),
		"Modifed", kAttrStatModified, B_TIME_TYPE);
	AddColInfo(colInfo);

	MenuDateColumn *created = new MenuDateColumn("Created", 200, 100, 300,
		B_ALIGN_LEFT);
	AddColumn(created, index++);
	colInfo = MakeColInfo(created, CalculateHash("Created", B_TIME_TYPE), "Created",
		kAttrStatCreated, B_TIME_TYPE);
	AddColInfo(colInfo);
	
	MenuStringColumn *kind = new MenuStringColumn("Kind", 100, 50, 200, B_ALIGN_LEFT);
	AddColumn(kind, index++);
	colInfo = MakeColInfo(kind, CalculateHash("Kind", B_STRING_TYPE), "Kind",
		kAttrMIMEType, B_STRING_TYPE);		

	MenuStringColumn *path = new MenuStringColumn("Path", 50, 100, 200,
		B_ALIGN_LEFT);
	AddColumn(path, index++);
	colInfo = MakeColInfo(path, CalculateHash("Path", B_STRING_TYPE), kAttrPath,
		"Path", B_STRING_TYPE);
	AddColInfo(colInfo);
	
	MenuStringColumn *perm = new MenuStringColumn("Permissions", 50, 100, 200,
		B_ALIGN_LEFT);
	AddColumn(perm, index++);
	colInfo = MakeColInfo(perm, CalculateHash("Permissions", B_STRING_TYPE),
		"Permissions", kAttrStatMode, B_STRING_TYPE);
	AddColInfo(colInfo);
	
	fAttrIndexOffset = index;
	
	SetSortingEnabled(true);
	SetSortColumn(name, false, true);
};

status_t QueryColumnListView::AddMIMEColumns(BMessage *msg) {
	status_t ret = B_OK;
	int32 index = fAttrIndexOffset;

	float maxWidthMulti = be_plain_font->StringWidth("M");

	for (int32 i = 0; msg->FindString("attr:name", i) != NULL; i++) {
		bool viewable = false;
		if (msg->FindBool("attr:viewable", i, &viewable) != B_OK) continue;
		if (viewable == false) continue;
		
		bool doAdd = false;
		const char *publicName;
		const char *internalName;
		alignment align = B_ALIGN_LEFT;
		int32 type;
		int32 widthTemp;
		float width = 0;
		float maxWidth = 0;
		float minWidth = 0;
		BColumn *column = NULL;

		if (msg->FindString("attr:name", i, &internalName) != B_OK) continue;
		if (msg->FindString("attr:public_name", i, &publicName) != B_OK) continue;
		if (msg->FindInt32("attr:type", i, &type) != B_OK) continue;
		msg->FindInt32("attr:alignment", i, (int32 *)&align);
		msg->FindInt32("attr:width", i, &widthTemp);
		width = widthTemp;
		maxWidth = width * maxWidthMulti;
		minWidth = be_plain_font->StringWidth(publicName);
					
		fAttributes[internalName] = publicName;
		fAttrTypes[internalName] = type;
		
		switch (type) {
			case B_CHAR_TYPE:
			case B_STRING_TYPE: {
				doAdd = true;
				column = new MenuStringColumn(publicName, width, minWidth, maxWidth,
					align);
			} break;
			
			case B_UINT8_TYPE:
			case B_INT8_TYPE:
			case B_UINT16_TYPE:
			case B_INT16_TYPE:
			case B_UINT32_TYPE:
			case B_INT32_TYPE:
			case B_UINT64_TYPE:
			case B_INT64_TYPE: {
				doAdd = true;
				column = new MenuIntegerColumn(publicName, width, minWidth,
					maxWidth, align);
			} break;
			
			case B_SIZE_T_TYPE: {
				doAdd = true;
				column = new MenuSizeColumn(publicName, width, minWidth,
					maxWidth, align);
			} break;
			
			case B_TIME_TYPE: {
				doAdd = true;
				column = new MenuDateColumn(publicName, width, minWidth,
					maxWidth, align);
			} break;
			
			default: {
				printf("%s (%s) is of an unhandled type: %4.4s\n", publicName,
					internalName, &type);
			};
		};
		
		if (doAdd) {
				col_info *colInfo = MakeColInfo(column, 0, publicName, internalName,
					B_SWAP_INT32(type));
				fInternalCols[internalName] = colInfo;
				fPublicCols[publicName] = colInfo;				

				fAttrIndex[internalName] = index;
				fIndexAttr[index] = internalName;
				AddColumn(column, index);
				index++;
		};
	};
	
	return ret;
};

uint32 QueryColumnListView::CalculateHash(const char *string, uint32 type) {
	char c;
	uint32 hash = 0;

	while((c = *string++) != 0) {
		hash = (hash << 7) ^ (hash >> 24);
		hash ^= c;
	}

	hash ^= hash << 12;

	hash &= ~0xff;
	hash |= type;

	return hash;
};

int32 QueryColumnListView::BackgroundAdder(void *arg) {
	QueryColumnListView *self = reinterpret_cast<QueryColumnListView *>(arg);
//	srand(system_time());
//	uint32 sleepTime = (rand() % 1000) * 1000;
	uint32 sleepTime = kSnoozePeriod;
	
	while (self->fSelfMsgr->IsValid()) {
		BMessage getPending(qclvRequestPending);
		BMessage pending;
#if B_BEOS_VERSION > B_BEOS_VERSION_5
		status_t request = self->fSelfMsgr->SendMessage(getPending, &pending);
#else
		status_t request = self->fSelfMsgr->SendMessage(&getPending, &pending);
#endif

		if (request == B_OK) {		
			entry_ref ref;
			bool updatedAny = false;
			
			if (pending.CountNames(B_ANY_TYPE) > 0) { 
				self->LockLooper();
				
				if (pending.FindBool("initial") == true) {
					entry_ref ref;
					querylist::iterator qIt;
					for (qIt = self->fQueries.begin(); qIt != self->fQueries.end(); qIt++) {
						BQuery *query = (*qIt);
						while (query->GetNextRef(&ref) == B_OK) {
							BNode node(&ref);
							result r;
				
							r.ref = ref;
							node.GetNodeRef(&r.nref);
							watch_node(&r.nref, B_WATCH_ALL, self);

							self->fResults[ref] = r;
							pending.AddRef("ref", &ref);
						};
					};
					
					updatedAny = true;
				};			
								
				for (int32 i = 0; pending.FindRef("ref", i, &ref) == B_OK; i++) {
					updatedAny = true;
					self->AddRowByRef(&ref);
				};
	
				if (updatedAny) {
					self->Hide();
					self->Sync();
					self->Show();
					self->Sync();
					
					if ((self->fNotify) && (self->fNotify->IsValid()) && (self->fMessage)) {
						BMessage send(*self->fMessage);
						send.AddInt32("qclvCount", self->fResults.size());
						
#if B_BEOS_VERSION > B_BEOS_VERSION_5
						self->fNotify->SendMessage(send);
#else
						self->fNotify->SendMessage(&send);
#endif
					};
				};
				self->UnlockLooper();
			};
		};
		
		snooze(sleepTime);
	};

	return B_OK;
};

status_t QueryColumnListView::ExtractPredicate(entry_ref *ref, char **buffer,
	int32 *length) {
	
	status_t ret = B_ERROR;
	if (*buffer) free(*buffer);
	*buffer = ReadAttribute(BNode(ref), kTrackerQueryPredicate, length);
	
	if ((*buffer != NULL) && (length > 0)) ret = B_OK;
	
	return ret;
};

status_t QueryColumnListView::ExtractVolumes(entry_ref *ref, vollist *vols) {
	BNode node(ref);
	int32 length = 0;
	char *attr = ReadAttribute(node, kTrackerQueryVolume, &length);
	BVolumeRoster roster;

	if (attr == NULL) {
		roster.Rewind();
		BVolume vol;
		
		while (roster.GetNextVolume(&vol) == B_NO_ERROR) {
			if (vol.KnowsQuery() == true) vols->push_back(vol);
		};
	} else {
		BMessage msg;
		msg.Unflatten(attr);

//		!*YOINK*!d from that project... with the funny little doggie as a logo...
//		OpenTracker, that's it!
			
		time_t created;
		off_t capacity;
		
		for (int32 index = 0; msg.FindInt32("creationDate", index, &created) == B_OK;
			index++) {
			
			if ((msg.FindInt32("creationDate", index, &created) != B_OK)
				|| (msg.FindInt64("capacity", index, &capacity) != B_OK))
				return B_ERROR;
		
			BVolume volume;
			BString deviceName = "";
			BString volumeName = "";
			BString fshName = "";
		
			if (msg.FindString("deviceName", &deviceName) == B_OK
				&& msg.FindString("volumeName", &volumeName) == B_OK
				&& msg.FindString("fshName", &fshName) == B_OK) {
				// New style volume identifiers: We have a couple of characteristics,
				// and compute a score from them. The volume with the greatest score
				// (if over a certain threshold) is the one we're looking for. We
				// pick the first volume, in case there is more than one with the
				// same score.
				int foundScore = -1;
				roster.Rewind();
				while (roster.GetNextVolume(&volume) == B_OK) {
					if (volume.IsPersistent() && volume.KnowsQuery()) {
						// get creation time and fs_info
						BDirectory root;
						volume.GetRootDirectory(&root);
						time_t cmpCreated;
						fs_info info;
						if (root.GetCreationTime(&cmpCreated) == B_OK
							&& fs_stat_dev(volume.Device(), &info) == 0) {
							// compute the score
							int score = 0;
		
							// creation time
							if (created == cmpCreated)
								score += 5;
							// capacity
							if (capacity == volume.Capacity())
								score += 4;
							// device name
							if (deviceName == info.device_name)
								score += 3;
							// volume name
							if (volumeName == info.volume_name)
								score += 2;
							// fsh name
							if (fshName == info.fsh_name)
								score += 1;
		
							// check score
							if (score >= 9 && score > foundScore) {
								vols->push_back(volume);
							}
						}
					}
				}
			} else {
				// Old style volume identifiers: We have only creation time and
				// capacity. Both must match.
				roster.Rewind();
				while (roster.GetNextVolume(&volume) == B_OK)
					if (volume.IsPersistent() && volume.KnowsQuery()) {
						BDirectory root;
						volume.GetRootDirectory(&root);
						time_t cmpCreated;
						root.GetCreationTime(&cmpCreated);
						if (created == cmpCreated && capacity == volume.Capacity()) {
							vols->push_back(volume);
						}
					}
			}
		};
	};

	return B_OK;	
};

entry_ref QueryColumnListView::ActionFor(entry_ref *ref) {
	entry_ref action;
	BPath queryPath(&fRef);
	BPath actionPath("/boot/home/config/settings/BeClan/QueryViewer/Actions");
	BPath tempPath = actionPath;
	
	tempPath.Append(queryPath.Leaf());
	tempPath.Append("_DEFAULT_");
	
	BEntry actionEntry(tempPath.Path(), false);
	if (actionEntry.Exists() == false) {
	
		tempPath = actionPath;
		char *mimeType = MIMETypeFor(ref);
		BString mime = mimeType;
		free(mimeType);
		mime.ReplaceAll("/", "_");
		tempPath.Append(mime.String());
		tempPath.Append("_DEFAULT_");

		actionEntry = BEntry(tempPath.Path(), false);

		if (actionEntry.Exists() == false) {
			tempPath = actionPath;
			tempPath.Append("Generic/_DEFAULT");

			actionEntry = BEntry(tempPath.Path(), false);
			if (actionEntry.Exists() == false) {
				actionEntry = BEntry("/boot/beos/system/Tracker", false);
			};
		};
	};
	
	actionEntry.GetRef(&action);
		
	return action;
};
