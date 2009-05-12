/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *		Michael Davidson <slaad@bong.com.au>
 */

#include <stdlib.h>
#include <stdio.h>

#include <interface/Alert.h>
#include <interface/Button.h>
#include <interface/OutlineListView.h>
#include <interface/ScrollView.h>
#include <interface/StringView.h>
#ifdef __HAIKU__
#	include <interface/GroupLayout.h>
#	include <interface/GroupLayoutBuilder.h>
#	include <interface/GridLayoutBuilder.h>
#endif

#include <storage/Path.h>
#include <support/String.h>

#include "PAccountsView.h"
#include "PAccountDialog.h"
#include "SettingsHost.h"

#include <libim/Helpers.h>
#include "common/GenericStore.h"
#include "common/Divider.h"

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

class AccountStore : public IM::GenericMapStore<BString, BMessage *> {
	public:
		AccountStore(const char *protocol)
			: IM::GenericMapStore<BString, BMessage *>(),
			fProtocol(protocol) {
		};
		
		status_t Save(void) {
			BMessage list;
			status_t result = im_protocol_get_account_list(fProtocol.String(), &list);

			if (result != B_OK) {
				return result;
			};

			BString account;

			for (int32 i = 0; list.FindString("account", i, &account) == B_OK; i++) {
				if (!Contains(account)) {
					result = im_protocol_delete_account(fProtocol.String(), account.String());
					if (result != B_OK) {
						return result;
					}
				};
			};

			for (Iterator it = Start(); it != End(); it++) {
				result = im_protocol_add_account(fProtocol.String(), it->first.String(), it->second);
				if (result != B_OK)
					return result;
			};

			return B_OK;
		};
		
		status_t Load(void) {
			Clear();
		
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

PAccountsView::PAccountsView(BRect bounds, BPath *protoPath)
	: AbstractView(bounds, protoPath->Path(), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	fProtoPath(*protoPath)
{
	float inset = ceilf(be_plain_font->Size() * 0.7f);
	// Font for heading
	BFont headingFont(be_bold_font);
	headingFont.SetSize(headingFont.Size() * 1.2f);

	BRect frame(0, 0, 1, 1);
#ifndef __HAIKU__
	frame = Frame();
	frame.InsetBy(inset * 2, inset * 2);
#endif

	// Heading
	char *titleBuffer = new char[513];
	(void)snprintf(titleBuffer, 512, _T("%s protocol"), protoPath->Leaf());
	titleBuffer[512] = '\0';
	fHeadingLabel = new BStringView(frame, "ProtocolLabel", titleBuffer);
	fHeadingLabel->SetAlignment(B_ALIGN_LEFT);
	fHeadingLabel->SetFont(&headingFont);
	delete titleBuffer;

	// Heading fHeadingDivider
	fHeadingDivider = new Divider(frame, "ProtocolsDivider", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS);
	fHeadingDivider->ResizeToPreferred();

	// Create list view
	fAccountListView = new BOutlineListView(frame, "ProtocolList",
		B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	BMessage *selection = new BMessage(kProtocolListChanged);
	selection->AddString("protocol", protoPath->Path());
	fAccountListView->SetSelectionMessage(selection);
	fAccountListView->SetInvocationMessage(new BMessage(kEditAccount));

	// Create scroll bars
#ifdef __HAIKU__
	fScrollView = new BScrollView("ProtocolListScroll", fAccountListView, 0,
		0, false, true, B_FANCY_BORDER);
#else
	fScrollView = new BScrollView("ProtocolListScroll", fAccountListView,
		B_FOLLOW_ALL, 0, false, true, B_FANCY_BORDER);
#endif

	// Buttons
	fAddButton = new BButton(frame, "Add", _T("Add account" B_UTF8_ELLIPSIS), new BMessage(kAddAccount));
	fEditButton = new BButton(frame, "Edit", _T("Edit account" B_UTF8_ELLIPSIS), new BMessage(kEditAccount));
	fEditButton->SetEnabled(false);
	fDelButton = new BButton(frame, "Del", _T("Remove"), new BMessage(kDelAccount));
	fDelButton->SetEnabled(false);

#ifdef __HAIKU__
	fHeadingLabel->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fHeadingDivider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
	fScrollView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fAddButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fEditButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	fDelButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, inset)
		.Add(BGridLayoutBuilder(0.0f, 1.0f)
			.Add(fHeadingLabel, 0, 0, 2)
			.Add(fHeadingDivider, 0, 1, 2)
		)

		.AddGroup(B_HORIZONTAL, 2.0f)
			.Add(fScrollView, 10.0f)

			.AddGroup(B_VERTICAL, 2.0f)
				.Add(fAddButton)
				.Add(fEditButton)
				.Add(fDelButton)
				.AddGlue()
			.End()
		.End()
	);
#else
	AddChild(fHeadingLabel);
	AddChild(fHeadingDivider);

	AddChild(fScrollView);
	
	AddChild(fAddButton);
	AddChild(fEditButton);
	AddChild(fDelButton);
	
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

	fAccountListView->SetTarget(this);
}

void PAccountsView::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case kProtocolListChanged: {		
			int32 index = B_ERROR;
			BStringItem *item = NULL;
			
			msg->FindInt32("index", &index);
			
			if (index >= 0) {
				item = dynamic_cast<BStringItem *>(fAccountListView->ItemAt(index));
			}
			
			fEditButton->SetEnabled(item != NULL);
			fDelButton->SetEnabled(item != NULL);
		} break;

		case kEditAccount:	
		case kAddAccount: {
			BMessage save(kSaveAccount);
			BMessage cancel(kCancelAccount);
			const char *account = NULL;
			BMessage tmplate = SettingsTemplate();
			BMessage settings;

			BString title = _T("Add account");
			
			if (msg->what == kEditAccount) {
				title = _T("Edit account");

				int32 index = fAccountListView->CurrentSelection(0);
				if (index < 0) return;

				BStringItem *item = dynamic_cast<BStringItem *>(fAccountListView->ItemAt(index));
				if (item == NULL) {
					return;
				};

				title << ": " << item->Text();
				title << " (" << fProtoPath.Leaf() << ")";
				
				account = item->Text();

				settings = *(fSettings->Find(account));
				save.AddPointer("listitem", item);
			}
		
			PAccountDialog *dialog = new PAccountDialog(title.String(), fProtoPath.Leaf(), account, tmplate, settings, new BMessenger(this), save, cancel);
			dialog->Show();
		} break;

		case kDelAccount: {
			int32 index = fAccountListView->CurrentSelection(0);
			if (index < 0)
				return;
			
			BStringItem *item = dynamic_cast<BStringItem *>(fAccountListView->ItemAt(index));
			if (item == NULL)
				return;

			if (fSettings->Contains(item->Text()) == true) {
				fSettings->Remove(item->Text());
			};		
			
			fAccountListView->RemoveItem(item);
			delete item;
			
			fEditButton->SetEnabled(false);
			fDelButton->SetEnabled(false);
			
			fHost->ControllerModified(this);
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

			bool isEdit = ((originalName != NULL) && (strlen(originalName) > 0));
			bool nameChanged = (strcmp(originalName, name) != 0);

			// Check if this account name is already used
			if (fSettings->Contains(name) == true) {
				// If the account name is not the same as the original name (ie. an edit) throw an error
				if ((isEdit == false) || (nameChanged == true)) {
					BAlert *alert = new BAlert("Error Saving",
						_T("An account with this name already exists"), _T("OK"), NULL, NULL,
						B_WIDTH_AS_USUAL, B_WARNING_ALERT);
					alert->Go(NULL);
					
					return;
				};
				
				fSettings->Remove(name);
			};
			
			if (isEdit == true) {
				fSettings->Remove(originalName);
			};
			
			fSettings->Add(name, new BMessage(newSettings));

			if (isEdit == true) {
				if ((nameChanged == true) && (item != NULL)) {
					item->SetText(name);
				}
			} else {
				fAccountListView->AddItem(new BStringItem(name));
			};
			BMessenger(dialog).SendMessage(B_QUIT_REQUESTED);
			
			fHost->ControllerModified(this);
		} break;
		
		default: {
			BView::MessageReceived(msg);
		} break;
	};
};

//#pragma mark SettingsController Hooks

status_t PAccountsView::Init(SettingsHost *host) {
	fHost = host;

	return B_OK;
};

status_t PAccountsView::Save(const BMessage *tmplate, BMessage *settings) {
	status_t result = fSettings->Save();
	LoadSettings(false);
	
	return result;
};

status_t PAccountsView::Revert(const BMessage *tmplate) {
	LoadSettings(true);
	
	return B_OK;
};


//#pragma mark Public

BMessage PAccountsView::SettingsTemplate(void) {
	BMessage tmplate;
	im_load_protocol_template(fProtoPath.Path(), &tmplate);
	
	return tmplate;
};

//#pragma mark Private

#ifndef __HAIKU__

void PAccountsView::LayoutGUI(void) {
	font_height fh;
	BFont headingFont(be_bold_font);
	headingFont.GetHeight(&fh);
	float headingFontHeight = fh.ascent + fh.descent + fh.leading;
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	BRect frame = Bounds();
	frame.InsetBy(inset * 2, inset * 2);

	// Heading
	fHeadingLabel->ResizeToPreferred();
	BRect frameHeadingLabel = fHeadingLabel->Frame();
	
	BRect frameHeadingDivider = fHeadingDivider->Frame();
	fHeadingDivider->MoveTo(frameHeadingDivider.left, frameHeadingLabel.bottom + inset);
	frameHeadingDivider = fHeadingDivider->Frame();

	// Accounts Scroll View
	BRect frameScrollView = fScrollView->Frame();
	fScrollView->MoveTo(frameScrollView.left, frameHeadingDivider.bottom + inset);
	frameScrollView = fScrollView->Frame();

	// Calculate the widest button....
	fAddButton->ResizeToPreferred();
	BRect frameAddButton = fAddButton->Frame();
	fEditButton->ResizeToPreferred();
	BRect frameEditButton = fEditButton->Frame();
	fDelButton->ResizeToPreferred();
	BRect frameDelButton = fDelButton->Frame();

	// Width of widest...
	float buttonWidth = max_c(frameAddButton.Width(), max_c(frameEditButton.Width(), frameDelButton.Width()));
	
	// Add Button
	fAddButton->MoveTo(frame.right - inset - buttonWidth, frameScrollView.top);
	fAddButton->ResizeTo(buttonWidth, frameAddButton.Height());
	frameAddButton = fAddButton->Frame();

	// Edit Button
	fEditButton->MoveTo(frame.right - inset - buttonWidth, frameAddButton.bottom + inset);
	fEditButton->ResizeTo(buttonWidth, frameEditButton.Height());
	frameEditButton = fEditButton->Frame();
	
	// Delete Button
	fDelButton->MoveTo(frame.right - inset - buttonWidth, frameEditButton.bottom + inset);
	fDelButton->ResizeTo(buttonWidth, frameDelButton.Height());
	frameDelButton = fDelButton->Frame();
	
	// Adjust the size of the ScrollView now that we know the location of the buttons
	// XXX: I can't figure out what the height should *actually* be....
	fScrollView->ResizeTo(frame.right - inset - buttonWidth - inset - B_V_SCROLL_BAR_WIDTH, frame.bottom - inset * 5 - B_H_SCROLL_BAR_HEIGHT);
	frameScrollView = fScrollView->Frame();

	fAccountListView->ResizeTo(frameScrollView.Width() - B_V_SCROLL_BAR_WIDTH, frameScrollView.Height() - B_H_SCROLL_BAR_HEIGHT);
};

#endif

void PAccountsView::LoadSettings(bool reload) {
	if (reload == true) {
		fSettings->Load();
	};

	BListItem *item = NULL;
	while ((item = fAccountListView->RemoveItem(0L)) != NULL) {
		delete item;
	};

	for (map<BString, BMessage *>::iterator it = fSettings->Start(); it != fSettings->End(); it++) {
		BStringItem *item = new BStringItem(it->first.String());
		fAccountListView->AddItem(item);
	};
};
