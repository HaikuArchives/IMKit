/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <VolumeRoster.h>
#include <Volume.h>
#include <Query.h>
#include <Mime.h>
#include <Path.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>

#include <libim/Helpers.h>
#include <libim/Contact.h>

#include <common/IMKitUtilities.h>
#include <common/interface/IconUtils.h>

const float kMiniSize = B_MINI_ICON * 1.25f;
const float kLargeSize = B_LARGE_ICON * 1.25f;

#include "PeopleColumnListView.h"


PeopleColumnListView::PeopleColumnListView(const char* _name)
	: BColumnListView(BRect(0, 0, 1, 1), _name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW,
		B_FANCY_BORDER, false)
{
	float iconSize = kLargeSize;

	// Columns
	BBitmapColumn* icon = new BBitmapColumn("", iconSize, iconSize, iconSize,
		B_ALIGN_CENTER);
	icon->SetShowHeading(false);
	AddColumn(icon, 0);

	BStringColumn* name = new BStringColumn("Name", 200, 100, 300,
		B_TRUNCATE_END, B_ALIGN_LEFT);
	AddColumn(name, 1);

	BStringColumn* status = new BStringColumn("Status", 200, 100, 300,
		B_TRUNCATE_END, B_ALIGN_LEFT);
	AddColumn(status, 2);

	BBitmapColumn* photo = new BBitmapColumn("Photo", iconSize, iconSize, iconSize,
		B_ALIGN_CENTER);
	AddColumn(photo, 3);
}


void
PeopleColumnListView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_QUERY_UPDATE: {
				int32 opcode = B_ERROR;

				if (msg->FindInt32("opcode", &opcode) != B_OK)
					return;

				const char *name = NULL;

				switch (opcode) {
					case B_ENTRY_CREATED:
						// Add to contact list
						msg->FindString("name", &name);
						//handle.entry.set_name(name);
						//msg->FindInt64("directory", &handle.entry.directory);
						//msg->FindInt32("device", &handle.entry.device);
						//msg->FindInt64("node", &handle.node);
						break;
					case B_ENTRY_REMOVED:
						// Remove from contact list
						msg->FindString("name", &name);
						break;
				}

				printf("--- %s\n", name);
			} break;
		default:
			BColumnListView::MessageReceived(msg);
			break;
	}
}


void
PeopleColumnListView::Populate()
{
	BVolumeRoster roster;
	BVolume vol;

	float iconSize = kLargeSize;

	// Get protocols list
	BMessage protocols;
	im_get_protocol_list(&protocols);

	// Add a tab for each supported protocol
	BMessage protocol;
	for (int32 i = 0; protocols.FindMessage("protocol", i, &protocol) == B_OK; i++) {
		const char* path = NULL;
		const char* file = NULL;

		protocol.FindString("path", &path);
		protocol.FindString("file", &file);

		// Protocol add-on path
		BPath protocolPath(path);
		protocolPath.Append(file, true);

		// Query predicate
		BString predicate;
		predicate << "((IM:connections==\"*"
			<< file << ":*\") && (BEOS:TYPE==\"application/x-person\"))";

		// Protocol icon
		BBitmap* protocolIcon = ReadNodeIcon(protocolPath.Path(), B_LARGE_ICON, true);

		// Add row for protocol
		BRow* row = new BRow(iconSize);
		row->SetField(new BBitmapField(protocolIcon), 0);
		row->SetField(new BStringField(file), 1);
		row->SetField(new BStringField(NULL), 2);
		row->SetField(new BBitmapField(NULL), 3);
		AddRow(row);

		// Add protocol to the map
		fProtocolItems[file] = new ProtocolItem();
		fProtocolItems[file]->row = row;

		// Rewind volume roster
		roster.Rewind();

		// Add all people files
		while (roster.GetNextVolume(&vol) == B_OK) {
			if (vol.InitCheck() != B_OK || !vol.IsPersistent() || !vol.KnowsQuery())
				continue;

			// Create query
			BQuery* query = new BQuery();
			query->SetTarget(BMessenger(this));
			query->SetPredicate(predicate.String());
			query->SetVolume(&vol);
			query->Fetch();

			// Add query to protocol queries list
			(void)fProtocolItems[file]->queries.AddItem(query);

			entry_ref ref;
			while (query->GetNextRef(&ref) == B_OK) {
				// Contact object
				IM::Contact contact(ref);

				// Get contact icon
				BBitmap* icon = new BBitmap(BRect(0, 0, 16, 16), B_RGBA32);
				BNode node(&ref);
				if (BIconUtils::GetIcon(&node, BEOS_ICON_ATTRIBUTE, BEOS_MINI_ICON_ATTRIBUTE,
					BEOS_LARGE_ICON_ATTRIBUTE, B_MINI_ICON, icon) != B_OK) {
					delete icon;
					icon = NULL;
				}

				// Get buddy icon
				BBitmap* buddyIcon = contact.GetBuddyIcon(file, B_LARGE_ICON);

				BRow* contactRow = new BRow(iconSize);
				contactRow->SetField(new BBitmapField(icon), 0);
				contactRow->SetField(new BStringField(ref.name), 1);
				contactRow->SetField(new BStringField(NULL), 2);
				contactRow->SetField(new BBitmapField(buddyIcon), 3);
				AddRow(contactRow, row);
			}
		}
	}
}
