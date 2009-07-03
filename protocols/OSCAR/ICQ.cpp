#include "ICQ.h"
/*
	Written by Michael "slaad" Davidson based off docs from
		http://iserverd1.khstu.ru/oscar/
		
					and
		
		http://aimdoc.sf.net/
*/

#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <libim/Contact.h>

#include <UTF8.h>
#include <Entry.h>

#include "Buddy.h"

//#pragma mark Extern C

extern "C" {
	IM::Protocol * load_protocol() {
		return new ICQProtocol();
	}
};

//#pragma mark Constructor

ICQProtocol::ICQProtocol()
	: IM::Protocol( Protocol::MESSAGES | Protocol::SERVER_BUDDY_LIST
	| Protocol::AWAY_MESSAGES | Protocol::BUDDY_ICON | Protocol::OFFLINE_MESSAGES),
	  fThread(0) {
	
	fPassword = "";
	fUIN = "";
	fEncoding = 0xffff; // No conversion == UTF-8
	fManager = new OSCARManager(dynamic_cast<OSCARHandler *>(this), "icq");
};

ICQProtocol::~ICQProtocol() {
}

//#pragma mark IM::Protocol Hooks

status_t ICQProtocol::Init(BMessenger msgr) {
	fMsgr = msgr;
	LOG(fManager->Protocol(), liMedium, "ICQProtocol::Init() start");
	
	fManager->Run();
	
	return B_OK;
}

status_t ICQProtocol::Shutdown() {
	fManager->LogOff();
	if (fManager->Lock()) fManager->Quit();
	
	LOG(fManager->Protocol(), liMedium, "ICQProtocol::Shutdown() done");
		
	return B_OK;
}

status_t ICQProtocol::Process(BMessage * msg) {
	switch (msg->what) {
		case IM::MESSAGE: {
			int32 im_what = 0;
			
			msg->FindInt32("im_what", &im_what);
		
			switch (im_what) {
				case IM::SERVER_LIST_ADD_CONTACT: {
					entry_ref ref;
					if (msg->FindRef("contact", &ref) != B_OK) return B_ERROR;
										
					IM::Contact contact(&ref);
					grouplist_t groups;
					int32 groupCount = contact.CountGroups();
					
					for (int32 i = 0; i < groupCount; i++) {
						groups.push_back(contact.GroupAt(i));
					};
					
					return fManager->AddSSIBuddy(msg->FindString("id"), groups);
				} break;

				case IM::UNREGISTER_CONTACTS: {
					fManager->RemoveBuddy(msg->FindString("id"));
				} break;
				
				case IM::REGISTER_CONTACTS: {

					if (fManager->IsConnected() == false) break;
					
					type_code garbage;
					int32 count = 0;
					msg->GetInfo("id", &garbage, &count);
					
					if (count > 0) {
						list<char *> buddies;
						for (int i = 0; msg->FindString("id", i); i++) {
							const char * id = msg->FindString("id",i);
							buddies.push_back(strdup(id));
						};
						fManager->AddBuddies(buddies);
					} else {
						fManager->AddBuddy(msg->FindString("id"));
					};
				} break;
				
				case IM::SET_STATUS: {
					const char *status = NULL;
					if (msg->FindString("status", &status) != B_OK) {
						LOG(fManager->Protocol(), liHigh, "Status set to NULL!");
						return B_ERROR;
					};

					LOG(fManager->Protocol(), liMedium, "Set status to %s", status);
					
					if (strcmp(status, OFFLINE_TEXT) == 0) {
						fManager->LogOff();
					} else
					if (strcmp(status, AWAY_TEXT) == 0) {
						if (fManager->ConnectionState() == (uchar)OSCAR_ONLINE) {
							const char *away_msg;
							if (msg->FindString("away_msg", &away_msg) == B_OK) {
								LOG(fManager->Protocol(), liMedium, "Setting away message: %s", away_msg);
								fManager->SetAway(away_msg);
							};
						};
					} else
					if (strcmp(status, ONLINE_TEXT) == 0) {
						if (fManager->IsConnected() == OSCAR_AWAY) {
							fManager->SetAway(NULL);
						} else {
							LOG(fManager->Protocol(), liDebug, "Calling fManager.Login()");
							fManager->Login("login.icq.com", (uint)5190,
								fUIN.String(), fPassword.String());
						};
					} else {
						LOG(fManager->Protocol(), liHigh, "Invalid status when setting status: '%s'", status);
					}
				} break;

				case IM::GET_CONTACT_INFO: {
					LOG(fManager->Protocol(), liLow, "Getting contact info", msg);
					const char * id = NormalizeNick(msg->FindString("id")).String();
					
					BMessage *infoMsg = new BMessage(IM::MESSAGE);
					Buddy *buddy = fManager->GetBuddy(id);
					
					infoMsg->AddInt32("im_what", IM::CONTACT_INFO);
					infoMsg->AddString("protocol", fManager->Protocol());
					infoMsg->AddString("id", id);
					infoMsg->AddString("nick", id);
					infoMsg->AddString("first name", id);
					if (buddy) infoMsg->AddBool("mobileuser", buddy->IsMobileUser());

					msg->SendReply(infoMsg);


					fMsgr.SendMessage(infoMsg);
				} break;
		
				case IM::SEND_MESSAGE: {
					const char *message_text = msg->FindString("message");
					BString srcid = msg->FindString("id");
					BString normal = NormalizeNick(srcid.String());
					BString screen = GetScreenNick(normal.String());
					
					const char *id = screen.String();
					
					LOG(fManager->Protocol(), liDebug, "SEND_MESSAGE (%s, %s)", msg->FindString("id"), msg->FindString("message"));
					LOG(fManager->Protocol(), liDebug, "  %s > %s > %s", srcid.String(), normal.String(), screen.String() );
					
					if (id == NULL) return B_ERROR;
					if (message_text == NULL) return B_ERROR;
					
					fManager->MessageUser(id, message_text);
					
					BMessage newMsg(*msg);
					newMsg.RemoveName("contact");
					newMsg.ReplaceInt32("im_what", IM::MESSAGE_SENT);
					
					fMsgr.SendMessage(&newMsg);
					
				} break;
				
				case IM::USER_STARTED_TYPING: {
					const char *id = msg->FindString("id");
					if (id == NULL) return B_ERROR;
				
					fManager->TypingNotification(id, STARTED_TYPING);
				} break;
				
				case IM::USER_STOPPED_TYPING: {
					const char *id = msg->FindString("id");
					if (id == NULL) return B_ERROR;
					
					fManager->TypingNotification(id, FINISHED_TYPING);
				} break;
				
				case IM::GET_AWAY_MESSAGE: {
					const char *id = msg->FindString("id");
					if (id == NULL) return B_ERROR;
				} break;
				
//				case IM::SEND_AUTH_ACK: {
//					bool authreply = false;
//					const char *id = msg->FindString("id");
//					int32 button = msg->FindInt32("which");
//					
//					if (button == 0) {
//						LOG("icq", liDebug, "Authorization granted to %s", id);
//						authreply = true;												
//					} else {
//						LOG("icq", liDebug, "Authorization rejected to %s", id);
//						authreply = false;					
//					}
//						
//					ICQ2000::ContactRef c = new ICQ2000::Contact( atoi(id) );
//					
//					AuthAckEvent * ev = new AuthAckEvent(c, authreply);
//
//					fClient.icqclient.SendEvent( ev );									
//					
//					if (authreply) {
//						// Create a new contact now that we authorized him/her/it.
//						BMessage im_msg(IM::MESSAGE);
//						im_msg.AddInt32("im_what", IM::CONTACT_AUTHORIZED);
//						im_msg.AddString("protocol", "icq");
//						im_msg.AddString("id", id);
//						im_msg.AddString("message", "" );
//						//im_msg.AddInt32("charset",fEncoding);
//	
//						fMsgr.SendMessage(&im_msg);
//					}
//
//				} break;
				
				default: {
				} break;
			};
		} break;
		
		default: {
		} break;
	};
	
	return B_OK;
};

const char * ICQProtocol::GetSignature() {
	return fManager->Protocol();
}

const char * ICQProtocol::GetFriendlySignature() {
	return "ICQ";
}

status_t ICQProtocol::UpdateSettings( BMessage & msg ) {
	const char *uin = NULL;
	const char *password = NULL;
	const char *profile = NULL;
	const char *iconPath = NULL;
	
	msg.FindString("uin", &uin);
	password = msg.FindString("password");
	profile = msg.FindString("profile");
	iconPath = msg.FindString("icon");
		
	if (uin == NULL || password == NULL || profile == NULL) {
		return B_ERROR;
	};
	
	fUIN = uin;
	fPassword = password;
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

uint32 ICQProtocol::GetEncoding(void) {
	return fEncoding;
}

//#pragma mark OSCARHandler hooks

status_t ICQProtocol::Error(const char *error) {
	BMessage msg(IM::ERROR);
	msg.AddString("protocol", fManager->Protocol());
	msg.AddString("error", error);
	
	return fMsgr.SendMessage(&msg);
};

status_t ICQProtocol::Progress(const char *id, const char *message, float progress) {
	BString progId = GetSignature();
	progId << id;

	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::PROGRESS );
	msg.AddString("protocol", fManager->Protocol());
	msg.AddString("progressID", progId);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", IM::impsConnecting );
	
	fMsgr.SendMessage(&msg);
	
	return B_OK;
};


status_t ICQProtocol::StatusChanged(const char *nick, online_types status,
	bool mobileUser = false) {
	BMessage msg(IM::MESSAGE);
	msg.AddString("protocol", fManager->Protocol());
	msg.AddBool("mobileuser", mobileUser);

	if (fUIN == nick) {
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

status_t ICQProtocol::MessageFromUser(const char *nick, const char *msg,
	bool autoReply = false) {
	
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddInt32("im_what", IM::MESSAGE_RECEIVED);
	im_msg.AddString("protocol", fManager->Protocol());
	im_msg.AddString("id", NormalizeNick(nick));
	
	BString text = msg;
	if (autoReply) text.Prepend("(Auto-response): ");
	text.ReplaceAll("\r", "");
		
	im_msg.AddString("message", text);
	im_msg.AddBool("autoresponse", autoReply);
	im_msg.AddInt32("charset", fEncoding);
	
	fMsgr.SendMessage(&im_msg);											

	return B_OK;
};

status_t ICQProtocol::UserIsTyping(const char *nick, typing_notification type) {
	BMessage im_msg(IM::MESSAGE);
	im_msg.AddString("protocol", fManager->Protocol());
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

status_t ICQProtocol::SSIBuddies(list<BString> buddies) {
	list <BString>::iterator i;

	BMessage serverBased(IM::SERVER_BASED_CONTACT_LIST);
	serverBased.AddString("protocol", fManager->Protocol());

	for (i = buddies.begin(); i != buddies.end(); i++) {
		LOG(fManager->Protocol(), liLow, "Got server side buddy %s", NormalizeNick(i->String()).String());
		serverBased.AddString("id", NormalizeNick(i->String()));
	};
			
	fMsgr.SendMessage(&serverBased);
};

status_t ICQProtocol::BuddyIconFromUser(const char *nick, const uchar *icon, uint32 length) {
	
	BMessage iconMsg(IM::MESSAGE);
	iconMsg.AddString("protocol", fManager->Protocol());
	iconMsg.AddInt32("im_what", IM::SET_BUDDY_ICON);
	iconMsg.AddString("id", NormalizeNick(nick));
	iconMsg.AddData("icondata", B_RAW_TYPE, icon, length);

	fMsgr.SendMessage(&iconMsg);
	return B_OK;
};

status_t ICQProtocol::AuthRequestFromUser(char *nick, char *reason) {
	BMessage msg(IM::MESSAGE);
	msg.AddInt32("im_what", IM::AUTH_REQUEST);
	msg.AddString("protocol", fManager->Protocol());
	msg.AddString("id", NormalizeNick(nick));
	msg.AddString("message", reason);
	
	return fMsgr.SendMessage(&msg);
};


char *ICQProtocol::RoastPassword(const char *pass) {
	int32 passLen = strlen(pass);
	char *ret = (char *)calloc(passLen + 1, sizeof(char));

	char encoding_table[] = {
		0xf3, 0x26, 0x81, 0xc4,
		0x39, 0x86, 0xdb, 0x92,
		0x71, 0xa3, 0xb9, 0xe6,
		0x53, 0x7a, 0x95, 0x7c
	};

	// encode the password
	for(int32 i = 0; i < passLen; i++) ret[i] = (pass[i] ^ encoding_table[i]);
	ret[passLen] = '\0';

	return ret;
};

void ICQProtocol::FormatMessageText(BString &message) {
};

//#pragma mark Private

BString ICQProtocol::NormalizeNick(const char *nick) {
	BString normal = nick;
	
	normal.ReplaceAll(" ", "");
	normal.ToLower();
	
	map<string,BString>::iterator i = fNickMap.find(normal.String());
	
	if ( i == fNickMap.end() ) {
		// add 'real' nick if it's not already there
		LOG(fManager->Protocol(), liDebug, "Adding normal (%s) vs screen (%s)", normal.String(), nick );
		fNickMap[string(normal.String())] = BString(nick);
	}
	
	LOG(fManager->Protocol(), liDebug, "Screen (%s) to normal (%s)", nick, normal.String() );
	
	return normal;
};

BString ICQProtocol::GetScreenNick( const char *nick ) {
	map<string,BString>::iterator i = fNickMap.find(nick);
	
	if ( i != fNickMap.end() ) {
		// found the nick
		LOG(fManager->Protocol(), liDebug, "Converted normal (%s) to screen (%s)", nick, (*i).second.String() );
		return (*i).second;
	}
	
	LOG(fManager->Protocol(), liDebug, "Nick (%s) not found in fNickMap, not converting", nick );
	
	return BString(nick);
};
