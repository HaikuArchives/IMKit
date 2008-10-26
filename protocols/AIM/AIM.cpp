#include "AIM.h"
/*
	Written by Michael "slaad" Davidson based off docs from
		http://iserverd1.khstu.ru/oscar/
		
					and
		
		http://aimdoc.sf.net/
*/

#include <libim/Constants.h>
#include <libim/Helpers.h>

#include <UTF8.h>

extern "C" {
	IM::Protocol * load_protocol() {
		return new AIMProtocol();
	}
};

AIMProtocol::AIMProtocol()
	: IM::Protocol( Protocol::MESSAGES | Protocol::SERVER_BUDDY_LIST
	| Protocol::AWAY_MESSAGES | Protocol::BUDDY_ICON),
	  fThread(0) {
	
	fPassword = NULL;
	fScreenName = NULL;
	fEncoding = 0xffff; // No conversion == UTF-8
	fManager = new AIMManager(dynamic_cast<OSCARHandler *>(this));

#if 0
	char buffer[] = {
		0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0xa3, 0xcf, 0xa1, 0x9e, 0x7b, 0x1d, 0x8b, 0x4f, 0x63, 0x6f,
		0x8b, 0xf4, 0x00, 0x01, 0x0b, 0x4d, 0x72, 0x20, 0x37, 0x33, 0x20, 0x43, 0x61, 0x70, 0x72, 0x69,
		0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x10, 0x00, 0x0f, 0x00, 0x04, 0x00, 0x00,
		0x00, 0x7e, 0x00, 0x1d, 0x00, 0x14, 0x00, 0x01, 0x01, 0x10, 0x16, 0x16, 0x77, 0xfc, 0xc2, 0xd8,
		0xc2, 0xe1, 0x98, 0xae, 0x3a, 0x9f, 0x18, 0xbe, 0x69, 0x0e, 0x00, 0x03, 0x00, 0x04, 0x43, 0xf6,
		0x8e, 0x29, 0x00, 0x13, 0x00, 0x01, 0x17, 0x00, 0x02, 0x00, 0xa9, 0x05, 0x01, 0x00, 0x01, 0x01,
		0x01, 0x01, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x68, 0x74, 0x6d, 0x6c, 0x3e, 0x3c, 0x62,
		0x6f, 0x64, 0x79, 0x20, 0x69, 0x63, 0x68, 0x61, 0x74, 0x62, 0x61, 0x6c, 0x6c, 0x6f, 0x6f, 0x6e,
		0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3d, 0x22, 0x23, 0x45, 0x38, 0x41, 0x36, 0x33, 0x30, 0x22, 0x20,
		0x69, 0x63, 0x68, 0x61, 0x74, 0x74, 0x65, 0x78, 0x74, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3d, 0x22,
		0x23, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x22, 0x3e, 0x3c, 0x66, 0x6f, 0x6e, 0x74, 0x20, 0x66,
		0x61, 0x63, 0x65, 0x3d, 0x22, 0x48, 0x65, 0x6c, 0x76, 0x65, 0x74, 0x69, 0x63, 0x61, 0x22, 0x20,
		0x41, 0x42, 0x53, 0x5a, 0x3d, 0x31, 0x32, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3d, 0x22, 0x23,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x22, 0x3e, 0x69, 0x74, 0x27, 0x73, 0x20, 0x6c, 0x69, 0x6b,
		0x65, 0x20, 0x4d, 0x53, 0x20, 0x43, 0x6f, 0x6d, 0x69, 0x63, 0x20, 0x43, 0x68, 0x61, 0x74, 0x3c,
		0x2f, 0x66, 0x6f, 0x6e, 0x74, 0x3e, 0x3c, 0x2f, 0x62, 0x6f, 0x64, 0x79, 0x3e, 0x3c, 0x2f, 0x68,
		0x74, 0x6d, 0x6c, 0x3e, 0x00, 0x0b, 0x00, 0x00
	};
	
	BMessage msg;
	msg.AddData("data", B_RAW_TYPE, buffer, sizeof(buffer));
	
	fManager->HandleICBM(&msg);
#endif
};

AIMProtocol::~AIMProtocol() {
	if (fScreenName) free(fScreenName);
	if (fPassword) free(fPassword);
}

status_t AIMProtocol::Init(BMessenger msgr) {
	fMsgr = msgr;
	LOG(kProtocolName, liMedium, "AIMProtocol::Init() start");
	
	fManager->Run();
	
	return B_OK;
}

status_t AIMProtocol::Shutdown() {
	fManager->LogOff();
	if (fManager->Lock()) fManager->Quit();
	
	LOG(kProtocolName, liMedium, "AIMProtocol::Shutdown() done");
		
	return B_OK;
}

status_t AIMProtocol::Process(BMessage * msg) {
	switch (msg->what) {
		case IM::MESSAGE: {
			int32 im_what=0;
			
			msg->FindInt32("im_what",&im_what);
		
			switch (im_what) {
				case IM::UNREGISTER_CONTACTS: {
					fManager->RemoveBuddy(msg->FindString("id"));
				} break;
				case IM::REGISTER_CONTACTS:
				{
					if ( !fManager->IsConnected() )
						break;
					
					type_code garbage;
					int32 count = 0;
					msg->GetInfo("id", &garbage, &count);
					
					if (count > 0) {
						list<char *> buddies;
						for ( int i=0; msg->FindString("id",i); i++ )
						{
							const char * id = msg->FindString("id",i);
							buddies.push_back(strdup(id));
						};
						fManager->AddBuddies(buddies);
					} else {
						fManager->AddBuddy(msg->FindString("id"));
					};
				}	break;
				
				case IM::SET_STATUS: {
					const char *status = NULL;
					if (msg->FindString("status", &status) != B_OK) {
						LOG(kProtocolName, liHigh, "Status set to NULL!");
						return B_ERROR;
					};

					LOG(kProtocolName, liMedium, "Set status to %s", status);
					
					if (strcmp(status, OFFLINE_TEXT) == 0) {
						fManager->LogOff();
					} else
					if (strcmp(status, AWAY_TEXT) == 0) {
						if (fManager->ConnectionState() == (uchar)OSCAR_ONLINE) {
							const char *away_msg;
							if (msg->FindString("away_msg", &away_msg) == B_OK) {
								LOG(kProtocolName, liMedium, "Setting away message: %s", away_msg);
								fManager->SetAway(away_msg);
							};
						};
					} else
					if (strcmp(status, ONLINE_TEXT) == 0) {
						if (fManager->IsConnected() == OSCAR_AWAY) {
							fManager->SetAway(NULL);
						} else {
							LOG(kProtocolName, liDebug, "Calling fManager.Login()");
							fManager->Login("login.oscar.aol.com", (uint16)5190,
								fScreenName, fPassword);
						};
					} else
					{
						LOG(kProtocolName, liHigh, "Invalid status when setting status: '%s'", status);
					}
				} break;

				case IM::GET_CONTACT_INFO:
				{
					LOG(kProtocolName, liLow, "Getting contact info", msg);
					const char * id = NormalizeNick(msg->FindString("id")).String();
					
					BMessage *infoMsg = new BMessage(IM::MESSAGE);
					infoMsg->AddInt32("im_what", IM::CONTACT_INFO);
					infoMsg->AddString("protocol", kProtocolName);
					infoMsg->AddString("id", id);
					infoMsg->AddString("nick", id);
					infoMsg->AddString("first name", id);
					//msg->AddString("last name", id);

					fMsgr.SendMessage(infoMsg);
				}	break;
		
				case IM::SEND_MESSAGE:
				{
					const char * message_text = msg->FindString("message");
					BString srcid = msg->FindString("id");
					BString normal = NormalizeNick(srcid.String());
					BString screen = GetScreenNick(normal.String());
					
					const char * id = screen.String();
					
					LOG(kProtocolName, liDebug, "SEND_MESSAGE (%s, %s)", msg->FindString("id"), msg->FindString("message"));
					LOG(kProtocolName, liDebug, "  %s > %s > %s", srcid.String(), normal.String(), screen.String() );
					
					if ( !id )
						return B_ERROR;
					
					if ( !message_text )
						return B_ERROR;
					
					fManager->MessageUser(id, message_text);
					
					BMessage newMsg(*msg);
					
					newMsg.RemoveName("contact");
					newMsg.ReplaceInt32("im_what", IM::MESSAGE_SENT);
					
					fMsgr.SendMessage(&newMsg);
					
				}	break;
				case IM::USER_STARTED_TYPING: {
					const char *id = msg->FindString("id");
					if (!id) return B_ERROR;
				
					fManager->TypingNotification(id, STARTED_TYPING);
				} break;
				case IM::USER_STOPPED_TYPING: {
					const char *id = msg->FindString("id");
					if (!id) return B_ERROR;
					
					fManager->TypingNotification(id, FINISHED_TYPING);
				} break;
				
				case IM::GET_AWAY_MESSAGE: {
					const char *id = msg->FindString("id");
					if (!id) return B_ERROR;
					
				};
				
				default:
					break;
			}
			
		}	break;
		default:
			break;
	}
	
	return B_OK;
}

const char * AIMProtocol::GetSignature() {
	return kProtocolName;
}

const char * AIMProtocol::GetFriendlySignature() {
	return "AIM";
}

BMessage AIMProtocol::GetSettingsTemplate() {
	BMessage main_msg(IM::SETTINGS_TEMPLATE);
	
	BMessage user_msg;
	user_msg.AddString("name","screenname");
	user_msg.AddString("description", "Screen Name");
	user_msg.AddInt32("type",B_STRING_TYPE);
	
	BMessage pass_msg;
	pass_msg.AddString("name","password");
	pass_msg.AddString("description", "Password");
	pass_msg.AddInt32("type",B_STRING_TYPE);
	pass_msg.AddBool("is_secret", true);
	
/*	BMessage enc_msg;
	enc_msg.AddString("name","encoding");
	enc_msg.AddString("description","Text encoding");
	enc_msg.AddInt32("type", B_STRING_TYPE);
	enc_msg.AddString("valid_value", "ISO 8859-1");
	enc_msg.AddString("valid_value", "UTF-8");
	enc_msg.AddString("valid_value", "JIS");
	enc_msg.AddString("valid_value", "Shift-JIS");
	enc_msg.AddString("valid_value", "EUC");
	enc_msg.AddString("valid_value", "Windows 1251");
	enc_msg.AddString("default", "ISO 8859-1");
*/

	BMessage profile;
	profile.AddString("name", "profile");
	profile.AddString("description", "User Profile");
	profile.AddInt32("type", B_STRING_TYPE);
	profile.AddString("default", "IM Kit: AIM user");
	profile.AddBool("multi_line", true);

	BMessage icon;
	icon.AddString("name", "icon");
	icon.AddString("description", "Buddy Icon");
	icon.AddInt32("type", B_STRING_TYPE);
		
	main_msg.AddMessage("setting",&user_msg);
	main_msg.AddMessage("setting",&pass_msg);
//	main_msg.AddMessage("setting",&enc_msg);
	main_msg.AddMessage("setting", &profile);
	main_msg.AddMessage("setting", &icon);
	
	return main_msg;
}

status_t AIMProtocol::UpdateSettings( BMessage & msg ) {
	const char * screenname = NULL;
	const char * password = NULL;
//	const char * encoding = NULL;
	const char *profile = NULL;
	const char *iconPath = NULL;
	
	msg.FindString("screenname", &screenname);
	password = msg.FindString("password");
//	encoding = msg.FindString("encoding");
	profile = msg.FindString("profile");
	iconPath = msg.FindString("icon");
		
	if ( screenname == NULL || password == NULL || profile == NULL)
		// invalid settings, fail
		return B_ERROR;
	
//	if ( strcmp(encoding,"ISO 8859-1") == 0 )
//	{
////		fEncoding = B_ISO1_CONVERSION;
//	} else
//	if ( strcmp(encoding,"UTF-8") == 0 )
//	{
////		fEncoding = 0xffff;
//	} else
//	if ( strcmp(encoding,"JIS") == 0 )
//	{
////		fEncoding = B_JIS_CONVERSION;
//	} else
//	if ( strcmp(encoding,"Shift-JIS") == 0 )
//	{
////		fEncoding = B_SJIS_CONVERSION;
//	} else
//	if ( strcmp(encoding,"EUC") == 0 )
//	{
////		fEncoding = B_EUC_CONVERSION;
//	} else
//	if ( strcmp(encoding,"Windows 1251") == 0 )
//	{
////		fEncoding = B_MS_WINDOWS_1251_CONVERSION;
//	} else
//	{ // invalid encoding value
//		return B_ERROR;
//	}
//	
	if (fScreenName) free(fScreenName);
	fScreenName = strdup(screenname);
	
	if (fPassword) free(fPassword);
	fPassword = strdup(password);
	
	fManager->SetProfile(profile);
	
	if (iconPath) {
		BFile file(iconPath, B_READ_ONLY);
		if (file.InitCheck() == B_OK) {
			off_t size;
			if (file.GetSize(&size) == B_OK) {
				char *buffer = (char *)calloc(size, sizeof(char));
				if (file.Read(buffer, size) == size) {
					fManager->SetIcon(buffer, size);
				};
				
				free(buffer);
			};
		};
		file.Unset();
	};
	
	return B_OK;
}

uint32 AIMProtocol::GetEncoding() 
{
	return fEncoding;
}

status_t AIMProtocol::Error(const char *error) {
	BMessage msg(IM::ERROR);
	msg.AddString("protocol", kProtocolName);
	msg.AddString("error", error);
	
	fMsgr.SendMessage(&msg);
};

status_t AIMProtocol::Progress(const char *id, const char *message,
	float progress) {

	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::PROGRESS );
	msg.AddString("protocol", kProtocolName);
	msg.AddString("progressID", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", IM::impsConnecting );
	
	fMsgr.SendMessage(&msg);
	
	return B_OK;
};


status_t AIMProtocol::StatusChanged(const char *nick, online_types status) {
	BMessage msg(IM::MESSAGE);
	msg.AddString("protocol", kProtocolName);

	if (strcmp(nick, fScreenName) == 0) {
		msg.AddInt32("im_what", IM::STATUS_SET);
	} else {
		msg.AddInt32("im_what", IM::STATUS_CHANGED);
		msg.AddString("id", NormalizeNick(nick));
	};

	switch (status) {
		case OSCAR_ONLINE: {
			msg.AddString("status", ONLINE_TEXT);
		} break;
		case OSCAR_AWAY: {
			msg.AddString("status", AWAY_TEXT);
		} break;
		case OSCAR_OFFLINE: {
			msg.AddString("status", OFFLINE_TEXT);
		} break;
		
		default: {
			return B_ERROR;
		};
	};

	fMsgr.SendMessage(&msg);
	
	return B_OK;
};

status_t AIMProtocol::MessageFromUser(const char *nick, const char *msg,
	bool autoReply = false) {
	
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::MESSAGE_RECEIVED);
	im_msg.AddString("protocol", kProtocolName);
	im_msg.AddString("id", NormalizeNick(nick));
	
	BString text = msg;
	if (autoReply) text.Prepend("(Auto-response): ");
		
	im_msg.AddString("message", text);
	im_msg.AddBool("autoresponse", autoReply);
	im_msg.AddInt32("charset", fEncoding);
	
	fMsgr.SendMessage(&im_msg);											

	return B_OK;
};

status_t AIMProtocol::UserIsTyping(const char *nick, typing_notification type) {
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddString("protocol", kProtocolName);
	im_msg.AddString("id", NormalizeNick(nick));

	switch (type) {
		case STILL_TYPING:
		case STARTED_TYPING: {
			im_msg.AddInt32("im_what", IM::CONTACT_STARTED_TYPING);
		} break;
		case FINISHED_TYPING:
		default: {
			im_msg.AddInt32("im_what", IM::CONTACT_STOPPED_TYPING);
		} break;
	};
	
	fMsgr.SendMessage(&im_msg);
	
	return B_OK;
};

status_t AIMProtocol::SSIBuddies(list<BString> buddies) {
	list <BString>::iterator i;

	BMessage serverBased(IM::SERVER_BASED_CONTACT_LIST);
	serverBased.AddString("protocol", kProtocolName);

	for (i = buddies.begin(); i != buddies.end(); i++) {
		LOG(kProtocolName, liLow, "Got server side buddy %s", NormalizeNick(i->String()).String());
		serverBased.AddString("id", NormalizeNick(i->String()));
	};
			
	fMsgr.SendMessage(&serverBased);
};

status_t AIMProtocol::BuddyIconFromUser(const char *nick, const uchar *icon,
	uint32 length) {
	
	BMessage iconMsg(IM::MESSAGE);
	iconMsg.AddString("protocol", kProtocolName);
	iconMsg.AddInt32("im_what", IM::SET_BUDDY_ICON);
	iconMsg.AddString("id", NormalizeNick(nick));
	iconMsg.AddData("icondata", B_RAW_TYPE, icon, length);

	fMsgr.SendMessage(&iconMsg);
	return B_OK;
};

//#pragma mark -

BString AIMProtocol::NormalizeNick(const char *nick) {
	BString normal = nick;
	
	normal.ReplaceAll(" ", "");
	normal.ToLower();
	
	map<string,BString>::iterator i = fNickMap.find(normal.String());
	
	if ( i == fNickMap.end() ) {
		// add 'real' nick if it's not already there
		LOG(kProtocolName, liDebug, "Adding normal (%s) vs screen (%s)", normal.String(), nick );
		fNickMap[string(normal.String())] = BString(nick);
	}
	
	LOG(kProtocolName, liDebug, "Screen (%s) to normal (%s)", nick, normal.String() );
	
	return normal;
};

BString AIMProtocol::GetScreenNick( const char *nick ) {
	map<string,BString>::iterator i = fNickMap.find(nick);
	
	if ( i != fNickMap.end() ) {
		// found the nick
		LOG(kProtocolName, liDebug, "Converted normal (%s) to screen (%s)", nick, (*i).second.String() );
		return (*i).second;
	}
	
	LOG(kProtocolName, liDebug, "Nick (%s) not found in fNickMap, not converting", nick );
	
	return BString(nick);
};
