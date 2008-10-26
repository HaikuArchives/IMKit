#include "IMInfoApp.h"

#include <stdio.h>
#include <Messenger.h>

IMInfoApp::IMInfoApp(void)
	: BApplication("application/x-vnd.beclan.IM_InfoPopper") {

	fManager = new IM::Manager(this);
	fManager->StartListening();
	
	BMessage settings;
	bool temp;
	im_load_client_settings("IM-InfoPopper", &settings);
	if ( !settings.FindString("app_sig") )
		settings.AddString("app_sig", "application/x-vnd.beclan.IM_InfoPopper");
	if ( settings.FindBool("auto_start", &temp) != B_OK )
		settings.AddBool("auto_start", true );
	if (settings.FindString("status_text", &fStatusText) != B_OK) {
		fStatusText = "$nickname$ ($protocol$:$id$) is now $status$";
		settings.AddString("status_text", fStatusText);
	};
	if (settings.FindString("msg_text", &fMessageText) != B_OK) {
		fMessageText = "$nickname$ says:\n$shortmsg$";
		settings.AddString("msg_text", fMessageText);
	};
/*	if (settings.FindInt32("icon_size", &fIconSize) != B_OK) {
		fIconSize = 16;
		settings.AddInt32("icon_size", fIconSize);
	};
*/	
	im_save_client_settings("im_infopopper", &settings);
	
	BDirectory dir("/boot/home/config/add-ons/im_kit/protocols");
	entry_ref ref;
	
	dir.Rewind();
	
	while (dir.GetNextRef(&ref) == B_OK) {
		BPath path(&ref);
//		BBitmap *icon = ReadNodeIcon(path.Path(), fIconSize);
		
		fProtocolIcons[path.Leaf()] = ref;
	};	
};

IMInfoApp::~IMInfoApp(void) {
	fManager->StopListening();
	BMessenger(fManager).SendMessage(B_QUIT_REQUESTED);
};

void IMInfoApp::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case IM::ERROR:
		case IM::MESSAGE: {
			int32 im_what = IM::ERROR;
			if (msg->FindInt32( "im_what", &im_what) != B_OK) im_what = IM::ERROR;
				
			BString text("");		
			entry_ref ref;
			
			msg->FindRef("contact", &ref);
			
			BString title;
			const char *protocol;
			char contactname[512];
			char nickname[512];
			char email[512];
			char status[512];
			BBitmap *icon = NULL;
			bool useProtoIcon = false;
			entry_ref protoRef;
			int16 iconSize = -1;

			if (im_what == IM::USER_STARTED_TYPING) return;
			if (im_what == IM::USER_STOPPED_TYPING) return;

			BMessage sizeReply;
			BMessenger(InfoPopperAppSig).SendMessage(InfoPopper::GetIconSize, &sizeReply);
			if (sizeReply.FindInt16("iconSize", &iconSize) != B_OK) iconSize = 48;
						
			IM::Contact contact(&ref);

			if (contact.GetNickname(status, sizeof(status)) != B_OK) {
				strcpy(status, "<unknown status>");
			};
//			Exit if user is blocked
			if (strcasecmp(status, "blocked") == 0) break;

			if (contact.GetName(contactname, sizeof(contactname) ) != B_OK ) {
				strcpy(contactname, "<unknown contact>");
			};
			if (contact.GetEmail(email, sizeof(email)) != B_OK) {
				strcpy(email, "<unknown email>");
			};
			if (contact.GetNickname(nickname, sizeof(nickname)) != B_OK) {
				strcpy(nickname, "<unknown nick>");
			};

			if (msg->FindString("protocol", &protocol) == B_OK) {
				icon = contact.GetBuddyIcon(protocol, iconSize);
				protoicons::iterator pIt = fProtocolIcons.find(protocol);
				if (pIt != fProtocolIcons.end()) protoRef = pIt->second;
				if (icon == NULL) {
					icon = contact.GetBuddyIcon("general", iconSize);
					if (icon == NULL) useProtoIcon = true;
				};
			};
			
			InfoPopper::info_type type = InfoPopper::Information;
						
			switch (im_what) {
				case IM::ERROR:
				{
					title = "Error";
					BMessage error;
					int32 error_what = -1;
					if ( msg->FindMessage("message", &error ) == B_OK )
					{
						error.FindInt32("im_what", &error_what);
					}
					
					if ( error_what != IM::USER_STARTED_TYPING && 
						error_what != IM::USER_STOPPED_TYPING )
					{ // we ignore errors due to typing notifications.
						text << "Error: " << msg->FindString("error");
						type = InfoPopper::Error;
					}
				}	break;
				
				case IM::MESSAGE_RECEIVED: {
					title = "Message Received";
					text = fMessageText;
					BString message = msg->FindString("message");
					BString shortMessage = message;
					
					if ( shortMessage.FindFirst("\n") >= 0 )
					{
						shortMessage.Truncate( shortMessage.FindFirst("\n") );
						shortMessage.Append("...");
					}
					
					if ( shortMessage.Length() > 60 ) {
						shortMessage.Truncate(57);
						shortMessage.Append("...");
					}
					
					text.ReplaceAll("$shortmsg$", shortMessage.String());
					text.ReplaceAll("$message$", message.String());
					
					type = InfoPopper::Important;
				}	break;
				
				case IM::STATUS_CHANGED: {
					title = "Status change";
					const char * new_status = msg->FindString("status");
					const char * old_status = msg->FindString("old_status");
					
					if ( new_status && old_status && strcmp(new_status,old_status) )
					{ // only show if total status has changed
						text = fStatusText;
						
						text.ReplaceAll("$status$", msg->FindString("status"));
					}
				}	break;
				
				case IM::PROGRESS: {
					title = "Progress";
					float progress;
					if (msg->FindFloat("progress", &progress) != B_OK) progress=0.0f;
					const char * messageID;
					if (msg->FindString("progressID", &messageID) != B_OK) messageID=NULL;
					const char * message;
					if (msg->FindString("message", &message) != B_OK) message=NULL;
					
					BMessage pop_msg(InfoPopper::AddMessage);
					pop_msg.AddString("appTitle", "IM Kit");
					pop_msg.AddString("title", title);
					pop_msg.AddInt8("type", InfoPopper::Progress);
					pop_msg.AddFloat("progress",progress);
					if (messageID) pop_msg.AddString("messageID",messageID);
					if (message) pop_msg.AddString("content",messageID);
					
					if (icon) {
						BMessage image;
						icon->Archive(&image);
						pop_msg.AddMessage("icon", &image);
						
						delete icon;
						
						pop_msg.AddRef("overlayIconRef", &protoRef);
						pop_msg.AddInt32("overlayIconType", InfoPopper::Attribute);
					} else if (useProtoIcon) {
						pop_msg.AddRef("iconRef", &protoRef);
						pop_msg.AddInt32("iconType", InfoPopper::Attribute);
					};
					
					BMessenger(InfoPopperAppSig).SendMessage(&pop_msg);
				} return; // yes, return. Progress is a special case.
			};
			
			text.ReplaceAll("\\n", "\n");
			text.ReplaceAll("$nickname$", nickname);
			text.ReplaceAll("$contactname$", contactname);
			text.ReplaceAll("$email$", email);
			text.ReplaceAll("$id$", msg->FindString("id"));
			text.ReplaceAll("$protocol$", msg->FindString("protocol"));
			
//			Message to display
			if ( text != "" ) {
				LOG("im_infopopper", liDebug, "Displaying message <%s>", text.String() );
				
				BMessage pop_msg(InfoPopper::AddMessage);
				pop_msg.AddString("appTitle", "IM Kit");
				pop_msg.AddString("title", title);
				pop_msg.AddString("content", text);
				pop_msg.AddInt8("type", (int8)type);
				
				if (icon) {
					BMessage image;
					icon->Archive(&image);
					pop_msg.AddMessage("icon", &image);
					
					delete icon;
					
					LOG("im_infopopper", liDebug, "Icon size: %.2f", icon->Bounds().Width());
					
					pop_msg.AddRef("overlayIconRef", &protoRef);
					pop_msg.AddInt32("overlayIconType", InfoPopper::Attribute);
				} else if (useProtoIcon) {
					pop_msg.AddRef("iconRef", &protoRef);
					pop_msg.AddInt32("iconType", InfoPopper::Attribute);
				};
				
				pop_msg.AddString("onClickApp", "application/x-person");
				pop_msg.AddRef("onClickRef", &ref);
				
				BMessenger(InfoPopperAppSig).SendMessage(&pop_msg);				
			};
		} break;
		
		case IM::SETTINGS_UPDATED: {	
			BMessage settings;
			im_load_client_settings("im_infopopper", &settings);
			if (settings.FindString("status_text", &fStatusText) != B_OK) {
				fStatusText = "$nickname$ is now $status$";
			};
			if (settings.FindString("msg_text", &fMessageText) != B_OK) {
				fMessageText = "$nickname$ says:\n$shortmsg$";
			};			
		} break;
		
		default: {
			BApplication::MessageReceived(msg);
		};
	};
};
