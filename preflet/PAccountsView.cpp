/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>

#include <interface/Alert.h>
#include <interface/Button.h>
#include <interface/OutlineListView.h>
#include <interface/ScrollView.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#endif

#include <storage/Path.h>
#include <support/String.h>

#include "PAccountsView.h"
#include "PAccountDialog.h"

#include <libim/Helpers.h>
#include "common/GenericStore.h"

#ifdef ZETA
#	include <app/Roster.h>
#	include <locale/Locale.h>
#else
#	define _T(str) (str)
#endif

//#pragma mark Constants

const uint32 kProtocolListChanged = 'Mplc';
const uint32 kAddAccount = 'Mada';
const uint32 kEditAccount = 'Meda';
const uint32 kDelAccount = 'Mdea';
const uint32 kSaveAccount = 'Msac';
const uint32 kCancelAccount = 'Mcac';

//#pragma mark AccountStore

class AccountStore : public IM::GenericStore<BString, BMessage> {
	public:
		AccountStore(const char *protocol)
			: IM::GenericStore<BString, BMessage>(),
			fProtocol(protocol) {
		};
		
		status_t Save(void) {
			BMessage list;
			status_t result = im_protocol_get_account_list(fProtocol.String(), &list);
			
			if (result == B_OK) {
				BString account;
				for (int32 i = 0; list.FindString("account", i, &account) == B_OK; i++) {
					if (Contains(account) == false) {
						result = im_protocol_delete_account(fProtocol.String(), account.String());
						if (result != B_OK) {
							break;
						};
					};
				};
				
				if (result == B_OK) {
					for (map<BString, BMessage *>::iterator it = Start(); it != End(); it++) {
						result = im_protocol_add_account(fProtocol.String(), it->first.String(), it->second);
						if (result != B_OK) break;
					};
				};
			};
		
			return B_ERROR;
		};
		
		status_t Load(void) {
			BMessage list;
			status_t result = im_protocol_get_account_list(fProtocol.String(), &list);
			
			if (result == B_OK) {
				BString account;

				for (int32 i = 0; list.FindString("account", i, &account) == B_OK; i++) {
					BMessage settings;
					
					result = im_protocol_get_account(fProtocol.String(), account.String(), &settings);
					if (result != B_OK) {
						break;
					};
					
					Add(account, new BMessage(settings));
				};
			};
			
			return result;
		};

	private:
		BString	fProtocol;
};

//#pragma mark Constructor

PAccountsView::PAccountsView(BRect bounds, BPath* protoPath)
	: BView(bounds, protoPath->Path(), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fProtoPath(*protoPath)
{
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Frame();
	frame.InsetBy(inset * 2, inset * 2);
#endif

	// Create list view
	fProtocolListView = new BOutlineListView(frame, "ProtocolList", B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	BMessage *selection = new BMessage(kProtocolListChanged);
	selection->AddString("protocol", protoPath->Path());
	fProtocolListView->SetSelectionMessage(selection);

	// Create scroll bars
	BScrollView* scrollView = new BScrollView("ProtocolListScroll", fProtocolListView, B_FOLLOW_ALL, 0, false, true, B_FANCY_BORDER);

	// Buttons
	fAddButton = new BButton(frame, "Add", _T("Add account" B_UTF8_ELLIPSIS), new BMessage(kAddAccount));
	fEditButton = new BButton(frame, "Edit", _T("Edit account" B_UTF8_ELLIPSIS), new BMessage(kEditAccount));
	fEditButton->SetEnabled(false);
	fDelButton = new BButton(frame, "Del", _T("Remove"), new BMessage(kDelAccount));
	fDelButton->SetEnabled(false);

#ifdef __HAIKU__
	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_HORIZONTAL, inset)
		.Add(scrollView)

		.AddGroup(B_VERTICAL, 2.0f)
			.Add(fAddButton)
			.Add(fEditButton)
			.Add(fDelButton)
			.AddGlue()
		.End()
	);
#else
	AddChild(scrollView);
	
	LayoutGUI();
#endif

	fSettings = new AccountStore(fProtoPath.Leaf());
	LoadSettings(true);
}

PAccountsView::~PAccountsView(void) {
	delete fSettings;
};

//#pragma mark BView Hooks

void PAccountsView::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif

	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fDelButton->SetTarget(this);

	fProtocolListView->SetTarget(this);
}

void PAccountsView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kProtocolListChanged: {		
			int32 index = B_ERROR;
			BStringItem *item = NULL;
			
			msg->FindInt32("index", &index);
			
			if (index >= 0) {
				item = dynamic_cast<BStringItem *>(fProtocolListView->ItemAt(index));
			}
			
			fEditButton->SetEnabled(item != NULL);
			fDelButton->SetEnabled(item != NULL);
		} break;

		case kDelAccount: {
			int32 index = fProtocolListView->CurrentSelection(0);
			if (index < 0) return;
			
			BStringItem *item = dynamic_cast<BStringItem *>(fProtocolListView->ItemAt(index));
			if (item == NULL) return;
	
			if (fSettings->Contains(item->Text()) == true) {
				fSettings->Remove(item->Text());
			};		
			
			fProtocolListView->RemoveItem(item);
			delete item;
			
			fEditButton->SetEnabled(false);
			fDelButton->SetEnabled(false);
		} break;

		case kEditAccount:	
		case kAddAccount: {
			BMessage save(kSaveAccount);
			BMessage cancel(kCancelAccount);
			const char *account = NULL;
			BMessage tmplate;
			BMessage settings;

			im_load_protocol_template(fProtoPath.Path(), &tmplate);

			BString title = _T("Add account");
			if (msg->what == kEditAccount) {
				int32 index = fProtocolListView->CurrentSelection(0);
				if (index < 0) return;
				
				BStringItem *item = dynamic_cast<BStringItem *>(fProtocolListView->ItemAt(index));
				if (item == NULL) return;

				title << ": " << item->Text();
				title << " (" << fProtoPath.Leaf() << ")";
				
				account = item->Text();

				settings = fSettings->Find(account);
				save.AddPointer("listitem", item);
			}
		
			PAccountDialog *dialog = new PAccountDialog(title.String(), fProtoPath.Leaf(), account, tmplate, settings, new BMessenger(this), save, cancel);
			dialog->Show();
		} break;
		
		case kSaveAccount:
		case kCancelAccount: {
			PAccountDialog *dialog = NULL;
			if (msg->FindPointer("source", reinterpret_cast<void **>(&dialog)) != B_OK) return;
						
			if (msg->what == kCancelAccount) {
				BMessenger(dialog).SendMessage(B_QUIT_REQUESTED);

				return;
			}
			
			const char *name = NULL;
			if (msg->FindString("name", &name) != B_OK) {
				BMessenger(dialog).SendMessage(B_QUIT_REQUESTED);
				
				return;
			}
			
			BMessage newSettings;
			if (msg->FindMessage("settings", &newSettings) != B_OK) {
				BMessenger(dialog).SendMessage(B_QUIT_REQUESTED);
				
				return;
			};

			const char *originalName = NULL;
			msg->FindString("original_name", &originalName);

			BStringItem *item = NULL;
			msg->FindPointer("listitem", reinterpret_cast<void **>(&item));

			bool isEdit = (originalName != NULL);
			bool nameChanged = (strcmp(originalName, name) != 0);

			// Check if this account name is already used
			if (fSettings->Contains(name) == true) {
				// If the account name is not the same as the original name (ie. an edit) throw an error
				if ((isEdit == false) || (nameChanged == true)) {
					BAlert *alert = new BAlert("Error Saving", _T("An account with this name already exists"), _T("Okay"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					alert->Go(NULL);
					
					return;
				};
				
				fSettings->Remove(name);
			};
			
			if (isEdit == true) {
				fSettings->Remove(originalName);
			};
			
			fSettings->Add(name, new BMessage(newSettings));

			if ((isEdit == true) && (nameChanged == true) && (item != NULL)) {
				if (item != NULL) {
					item->SetText(name);
				}
			} else {
				fProtocolListView->AddItem(new BStringItem(name));
			};
			BMessenger(dialog).SendMessage(B_QUIT_REQUESTED);
		} break;
		
		default: {
			BView::MessageReceived(msg);
		} break;
	};
};

//#pragma mark SettingsController Hooks

status_t PAccountsView::Save(BView *view, const BMessage *tmplate, BMessage *settings) {
	status_t result = fSettings->Save();
	LoadSettings(false);
	
	return result;
};

status_t PAccountsView::Revert(BView *view, const BMessage *tmplate) {
	LoadSettings(true);
	
	return B_OK;
};


//#pragma mark Private

#ifndef __HAIKU__

void PAccountsView::LayoutGUI(void) {
};

#endif

void PAccountsView::LoadSettings(bool reload) {
	if (reload == true) {
		fSettings->Load();
	};

	BListItem *item = NULL;
	while ((item = fProtocolListView->RemoveItem(0L)) != NULL) {
		delete item;
	};

	for (map<BString, BMessage *>::iterator it = fSettings->Start(); it != fSettings->End(); it++) {
		BStringItem *item = new BStringItem(it->first.String());
		fProtocolListView->AddItem(item);
	};
};
