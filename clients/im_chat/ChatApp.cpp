/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Alert.h>

#include "ChatApp.h"

#define _T(str) (str)

const char* kDefaultPeopleHandler = "application/x-vnd.Be-PEPL";


ChatApp::ChatApp()
	: BApplication("application/x-vnd.IMKit-Chat"),
	fMan(new IM::Manager(BMessenger(this))),
	fIsQuiting(false)
{
	// Add system sound
	add_system_beep_event(kImNewMessageSound, 0);

	// Start listening to server messages
	fMan->StartListening();
}


ChatApp::~ChatApp()
{
	fMan->Lock();
	fMan->Quit();
}


bool
ChatApp::QuitRequested()
{
	// First check if im_server is shutting down, and if it isn't ask
	// user if (s)he really wants to quit
	BMessage* msg = CurrentMessage();
	bool wasShortcut = false;
	if (msg->FindBool("shortcut", &wasShortcut) == B_OK) {
		BMessage msg(IM::IS_IM_SERVER_SHUTTING_DOWN), reply;
		if (fMan->SendMessage(&msg,&reply) == B_OK) {
			bool isShuttingDown = true;
			if (reply.FindBool("isShuttingDown", &isShuttingDown) == B_OK) {
				if (!isShuttingDown) {
					BAlert * alert = new BAlert("",
						_T("Do you really want to quit im_client and stop getting messages?"),
						_T("Yes"), _T("No"));

					int32 choice = alert->Go();
					if (choice == 1)
						return false;
				}
			}
		}
	}

	RunMap::const_iterator it = fRunViews.begin();
	for (; it != fRunViews.end(); ++it) {
		BString key = it->first;
		RunView *rv = it->second;

		if (rv != NULL) {
			BWindow* win = rv->Window();
			if (win != NULL) {
				LOG("im_client", liLow, "RunView for %s has a Parent(), Lock()ing and "
					"RemoveSelf()ing.", key.String());
				win->Lock();
				rv->RemoveSelf();
				win->Unlock();
			} else {
				LOG("im_client", liLow, "RunView for %s doesn't have a Parent()", key.String());
				rv->RemoveSelf();
			}

			delete rv;
		} else {
			LOG("im_client", liLow, "RunView for %s was already NULL", key.String());
		}
	}

	fIsQuiting = true;

	fMan->StopListening();

	return BApplication::QuitRequested();
}


bool
ChatApp::IsQuiting()
{
	return fIsQuiting;
}


void
ChatApp::RefsReceived(BMessage* msg)
{
	entry_ref ref;
	BNode node;
	attr_info info;
	bool hasValidRefs = false;
	bool hasInvalidRefs = false;

	BMessage passToDefault(B_REFS_RECEIVED);

	for (int32 i = 0; msg->FindRef("refs", i, &ref) == B_OK; i++) {
		node = BNode(&ref);
		char* type = ReadAttribute(node, "BEOS:TYPE");
		if (type != NULL && strcmp(type, "application/x-person") == 0) {
			if (node.GetAttrInfo("IM:connections", &info) == B_OK) {
				msg->AddRef("contact", &ref);
				hasValidRefs = true;
			} else {
				passToDefault.AddRef("refs", &ref);
				hasInvalidRefs = true;
			}
		} else
			LOG("im_client", liLow, "Got a ref that wasn't a People file");
		free(type);
	}

	if (hasInvalidRefs)
		be_roster->Launch(fPeopleHandler.String(), &passToDefault);

	if (!hasValidRefs)
		return;

	msg->what = IM::MESSAGE;
	msg->AddInt32("im_what", IM::MESSAGE_RECEIVED);
	msg->AddBool("user_opened", true);

	BMessenger(this).SendMessage(msg);
}


void
ChatApp::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case IM::SETTINGS_UPDATED: {
			// Tell all windows that the settings have been updated
			for (int32 i = 0; i < CountWindows(); i++) {
				BWindow* win = WindowAt(i);
				if (win) {
					BMessenger msgr(win);
					msgr.SendMessage(msg);
				}
			}

			if (msg->FindString("people_handler", &fPeopleHandler) != B_OK)
				fPeopleHandler = kDefaultPeopleHandler;

		} break;

		case IM::ERROR:
		case IM::MESSAGE: {
			entry_ref ref;
			
			// Skip messages not to specific contacts
			if (msg->FindRef("contact",&ref) != B_OK)
				return;

			int32 im_what=-1;
			
			msg->FindInt32("im_what",&im_what);
			
			switch ( im_what )
			{
				case IM::CONTACT_INFO:
				{ // handle contact info updates
					//LOG("sample_client", liDebug, "Got contact info:",msg);
					
					const char * first_name = msg->FindString("first name");
					const char * last_name = msg->FindString("last name");
					const char * email = msg->FindString("email");
					const char * nick = msg->FindString("nick");
					
					if ( first_name || last_name )
					{
						char full_name[256];
						
						full_name[0] = 0;
						
						if ( first_name )
							strcat(full_name, first_name);
						
						if ( first_name && last_name )
							strcat(full_name, " ");
							
						if ( last_name )
							strcat(full_name, last_name);
						
						if ( strlen( full_name ) > 0 )
							setAttributeIfNotPresent( ref, "META:name", full_name );
					}
					
					if ( email )
					{
						if ( strlen( email ) > 0 )
							setAttributeIfNotPresent( ref, "META:email", email );
					}
					
					if ( nick )
					{
						if ( strlen( nick ) > 0 )
							setAttributeIfNotPresent( ref, "META:nickname", nick );
					}
				}	return;
				
				case IM::STATUS_CHANGED:
				{ // need to forward there to client windows so they can rebuild their protocol menus
					for ( int i=0; i<CountWindows(); i++ )			
					{
						BWindow * win = WindowAt(i);
						BMessenger( win ).SendMessage(msg);
					}
				}	return;
				
				case IM::GET_CONTACT_INFO:
					// ignore these so we don't open new windows that aren't needed.
					return;
				
				default:
					break;
			}
			
			ChatWindow * win = findWindow(ref);
			BMessenger _msgr(win); // fix for Lock() failing, hopefully.
			
			if ( !win && (im_what == IM::MESSAGE_RECEIVED) )
			{ // open new window on message received or user request
				LOG("im_client", liMedium, "Creating new window to handle message");
				win = new ChatWindow(ref);//, fIconBarSize, fCommandSends);
				chat_windows.AddItem(win);
				_msgr = BMessenger(win);
				if ( _msgr.LockTarget() )
				{
					win->Show();
					win->SetFlags(win->Flags() ^ B_AVOID_FOCUS);
					BMessenger(win).SendMessage(msg);
					win->Unlock();
					
					bool user_opened=false;
					
					if ( msg->FindBool("user_opened",&user_opened) != B_OK )
					{ // play sound if not opened by user
						system_beep(kImNewMessageSound);
					} else {
//						Was opened by the user, so make focus
						win->Activate(true);
					};
				} else
				{
					LOG("im_client", liHigh, "This is a fatal error that should never occur. Lock fail on new win.");
				}
			} else
			{
				if (_msgr.LockTarget() )
				{
					BMessenger(win).SendMessage(msg);
					if ( win->IsMinimized() )
					{ // window is hidden, move to this workspace and show it
						win->SetWorkspaces( B_CURRENT_WORKSPACE );
						win->SetFlags(win->Flags() | B_AVOID_FOCUS);
						win->Minimize(false);
						win->SetFlags(win->Flags() ^ B_AVOID_FOCUS);
					}
					win->Unlock();
				} else
				{
					LOG("im_client", liHigh, "This is a fatal error that should never occur. Lock fail on old win.");
				}
			}
		}	break;
		default:
			BApplication::MessageReceived(msg);
	}
}


ChatWindow*
ChatApp::findWindow(entry_ref& ref)
{
	for (int32 i = 0; chat_windows.ItemAt(i) != NULL; i++) {
		ChatWindow* win = (ChatWindow*)chat_windows.ItemAt(i);
		if (win->handlesRef(ref))
			return win;
	}

	return NULL;
}


void
ChatApp::Flash(BMessenger msgr)
{
	printf("We should be teh flasher!\n");
	fMan->FlashDeskbar(msgr);
}


void
ChatApp::NoFlash(BMessenger msgr)
{
	fMan->StopFlashingDeskbar(msgr);
}


status_t
ChatApp::StoreRunView(const char* id, RunView* rv)
{
	LOG("im_client", liLow, "Setting RunView for %s to %p", id, rv);

	BString nick = id;
	fRunViews[nick] = rv;

	return B_OK;
}


RunView*
ChatApp::GetRunView(const char* id)
{
	BString nick = id;

	LOG("im_client", liLow, "RunView for %s is %p", id, fRunViews[nick]);
	
	return fRunViews[nick];
}
