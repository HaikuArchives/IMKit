#include "AIMManager.h"

#include <libim/Protocol.h>
#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <UTF8.h>
#include <ctype.h>
#include <string.h>

#include <openssl/md5.h>

#include "OSCARConnection.h"
#include "AIMBOSConnection.h"
#include "AIMReqConn.h"
#include "OSCARHandler.h"
#include "htmlparse.h"
#include "TLV.h"

#if B_BEOS_VERSION==B_BEOS_VERSION_5
//Not beautiful. An inline function should probably be used instead.
# define strlcpy(_dest,_src,_len) \
	do { \
		if( _dest && _src ) {\
			if(_len>0) {\
				strncpy(_dest,_src,_len-1); \
			} \
		} \
		*(_dest+_len-1)='\0'; \
	}while(0)
#endif		

const uint16 AIM_ERROR_COUNT = 0x18;
const char *kErrors[] = {
	"",
	"Invalid SNAC header",
	"Server rate limit exceeded",
	"Client rate limit exceeded",
	"Recipient is not logged in",
	"Requested service unavailable",
	"Requested service undefined",
	"An old SNAC was sent",
	"Command not supported by server",
	"Command not supprted by client",
	"Refused by client",
	"Reply too big",
	"Responses lost",
	"Request denied",
	"Malformed SNAC",
	"Insufficient rights",
	"Recipient blocked",
	"Sender too evil",
	"Receiver too evil",
	"User unavailable",
	"No match",
	"List overflow",
	"Request ambiguous",
	"Server queue full",
	"Not while on my watch ... err... AOL"
};


void PrintHex(const unsigned char* buf, size_t size, bool override = false) {
	if ((g_verbosity_level != liDebug) && (override == false)){
		// only print this stuff in debug mode
		return;
	}
	
	uint16 i = 0;
	uint16 j = 0;
	int breakpoint = 0;

	for(;i < size; i++) {
		fprintf(stdout, "%02x ", (unsigned char)buf[i]);
		breakpoint++;	

		if(!((i + 1)%16) && i) {
			fprintf(stdout, "\t\t");
			for(j = ((i+1) - 16); j < ((i+1)/16) * 16; j++)	{
				if(buf[j] < 30) {
					fprintf(stdout, ".");
				} else {
					fprintf(stdout, "%c", (unsigned char)buf[j]);
				};
			}
			fprintf(stdout, "\n");
			breakpoint = 0;
		}
	}
	
	if(breakpoint == 16) {
		fprintf(stdout, "\n");
		return;
	}

	for(; breakpoint < 16; breakpoint++) {
		fprintf(stdout, "   ");
	}
	
	fprintf(stdout, "\t\t");

	for(j = size - (size%16); j < size; j++) {
		if(buf[j] < 30) {
			fprintf(stdout, ".");
		} else {
			fprintf(stdout, "%c", (unsigned char)buf[j]);
		};
	}
	
	fprintf(stdout, "\n");
}

void MD5(const char *data, int length, char *result) {
	MD5state_st state;
	
	MD5_Init(&state);
	MD5_Update(&state, data, length);
	MD5_Final((uchar *)result, &state);
};

//#pragma mark -

AIMManager::AIMManager(OSCARHandler *handler) {	
	fConnectionState = OSCAR_OFFLINE;
	
	fHandler = handler;
	fOurNick = NULL;
	fProfile.SetTo("IMKit AIM");
	
	fIcon = NULL;
	fIconSize = -1;
};

AIMManager::~AIMManager(void) {
	free(fOurNick);
	if (fIcon) free(fIcon);

	LogOff();
}

//#pragma mark -

status_t AIMManager::ClearConnections(void) {
	LOG(kProtocolName, liLow, "%i pending connections to close",
		fPendingConnections.size());

	pfc_map::iterator pIt;
	for (pIt = fPendingConnections.begin(); pIt != fPendingConnections.end(); pIt++) {
		AIMReqConn *con = dynamic_cast<AIMReqConn *>(pIt->second);
		if (con == NULL) continue;
		BMessenger(con).SendMessage(B_QUIT_REQUESTED);
	};
	fPendingConnections.clear();


	LOG(kProtocolName, liLow, "%i used connections to close", fConnections.size());

	connlist::iterator it;
	for (it = fConnections.begin(); it != fConnections.end(); it++) {
		OSCARConnection *con = (*it);
		if (con == NULL) continue;

		BMessenger(con).SendMessage(B_QUIT_REQUESTED);
	};	
	fConnections.clear();
	
	return B_OK;
};

status_t AIMManager::ClearWaitingSupport(void) {
	flap_stack::iterator it;
	
	for (it = fWaitingSupport.begin(); it != fWaitingSupport.end(); it++) {
		Flap *f = (*it);
		delete f;
	};
	
	fWaitingSupport.clear();
	
	return B_OK;
};

char *AIMManager::EncodePassword(const char *pass) {
	int32 passLen = strlen(pass);
	char *ret = (char *)calloc(passLen + 1, sizeof(char));
	
	char encoding_table[] = {
		0xf3, 0xb3, 0x6c, 0x99,
		0x95, 0x3f, 0xac, 0xb6,
		0xc5, 0xfa, 0x6b, 0x63,
		0x69, 0x6c, 0xc3, 0x9f
	};

	// encode the password
	for(int32 i = 0; i < passLen; i++ ) ret[i] = (pass[i] ^encoding_table[i]);

	ret[passLen] = '\0';
		
	return ret;
};

status_t AIMManager::HandleServiceControl(BMessage *msg) {
	status_t ret = B_OK;
	ssize_t bytes = 0;
	const uchar *data = NULL;
	uint16 offset = 0;
	uint16 subtype = 0;
	uint16 flags = 0;

	msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
	subtype = (data[2] << 8) + data[3];
	flags = (data[4] << 8) + data[5];
	
	offset = 9;
	
	if (flags & 0x8000) {
		uint16 skip = (data[++offset] << 8) + data[++offset];
		offset += skip;
	};

	switch (subtype) {
		case OWN_ONLINE_INFO: {
//			For some reason we're getting this upon sending an icon upload request
//			We should be getting a EXTENDED_STATUS

			if ((fIcon) && (fIconSize > 0)) {
				Flap *icon = new Flap(SNAC_DATA);
				icon->AddSNAC(new SNAC(SERVER_STORED_BUDDY_ICONS, ICON_UPLOAD_REQ));
				icon->AddInt16(fSSIItems++);		// First icon
				icon->AddInt16(fIconSize);
				icon->AddRawData((uchar *)fIcon, fIconSize);
							
				Send(icon);
			};
		} break;

		case EXTENDED_STATUS: {
			while (offset < bytes) {
				uint16 notice = (data[++offset] << 8) + data[++offset];

				switch (notice) {
//					Official icon
					case 0x0000: {
						int16 length = (data[++offset] << 8) + data[++offset];
						offset += length;
					} break;
					case 0x0001: {
						offset++; // Flags
						int8 hashLen = data[++offset];
						uchar currentHash[hashLen];
						memcpy(currentHash, data + offset, hashLen);
						offset += hashLen;
						
						if ((fIcon) && (fIconSize > 0)) {
							uchar hash[hashLen];
							MD5((uchar *)fIcon, fIconSize, (uchar *)hash);
							
							if (memcmp(hash, currentHash, hashLen) != 0) {
								LOG(kProtocolName, liLow, "Server stored buddy "
									"icon is different to ours - uploading");
								Flap *upload = new Flap(SNAC_DATA);
								upload->AddSNAC(new SNAC(SERVER_STORED_BUDDY_ICONS,
									 ICON_UPLOAD_REQ));
								upload->AddInt16(fSSIItems++);	// Next SSI item
								upload->AddInt16(fIconSize);
								upload->AddRawData((uchar *)fIcon, fIconSize);
								
								Send(upload);
							};
						};
					} break;
				};
			};
			
		} break;
		
		case MOTD: {
			PrintHex(data, bytes, true);
			ret = B_OK;
		} break;
		
		default: {
			ret = kUnhandled;
		};
	};

	return ret;
};

status_t AIMManager::HandleICBM(BMessage *msg) {
	status_t ret = B_OK;
	ssize_t bytes = 0;
	const uchar *data = NULL;
	uint16 offset = 0;
	uint16 subtype = 0;
	uint16 flags = 0;

	msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
	subtype = (data[2] << 8) + data[3];
	flags = (data[4] << 8) + data[5];
	
	offset = 9;
	
	if (flags & 0x8000) {
		uint16 skip = (data[++offset] << 8) + data[++offset];
		offset += skip;
	};
	
	switch (subtype) {
		case ERROR: {
			LOG(kProtocolName, liHigh, "GOT SERVER ERROR 0x%x 0x%x", data[++offset],
				data[++offset]);
		} break;
		
		case MESSAGE_FROM_SERVER: {
			PrintHex(data, bytes, true);
		
			offset = 18;
			
			uint16 channel = (data[offset] << 8) + data[++offset];
			uint8 nickLen = data[++offset];
			char *nick = (char *)calloc(nickLen + 1, sizeof(char));

			memcpy(nick, (void *)(data + offset + 1), nickLen);
			nick[nickLen] = '\0';

			offset += nickLen;
			uint16 warning = (data[++offset] << 8) + data[++offset];
			uint16 TLVs = (data[++offset] << 8) + data[++offset];

			LOG(kProtocolName, liDebug, "AIMManager: %i TLVs in message", TLVs);

			offset += 1;

			// We don't use these TLVs, so just skip through them.
			for (uint16 i = 0; i < TLVs; i++) {
				TLV tlv(data + offset, bytes - offset);
				offset += tlv.FlattenedSize();
			};
			
//			We only support plain text channels currently									
			switch (channel) {
				case PLAIN_TEXT: {
					int16 msgLen = 0;
					char *message = NULL;
					bool autoReply = false;
					
					while (offset < bytes) {
						TLV tlv(data + offset, bytes - offset);
						switch (tlv.Type()) {
							case 0x0002: {	// The message holding TLV
								message = ParseMessage(&tlv);
								offset += tlv.FlattenedSize();
							} break;
							
							case 0x0004: {
								autoReply = true;
								offset += tlv.FlattenedSize();
							} break;
							
							default: {
								offset += tlv.FlattenedSize();
							};
						};
					};

					if (message) {
						parse_html(message);
												
						// We just got a message from the user, they must
						// be online (This allows us to reply)
						fHandler->StatusChanged(nick, OSCAR_ONLINE);
						fHandler->MessageFromUser(nick, message, autoReply);
	
						free(message);
					};
				} break;
			} // end switch(channel)
			
			free(nick);
							
		} break;
		
		case TYPING_NOTIFICATION: {
			offset += 8;
			uint16 channel = (data[++offset] << 8) + data[++offset];
			uint8 nickLen = data[++offset];
			char *nick = (char *)calloc(nickLen + 1, sizeof(char));
			memcpy(nick, (char *)(data + offset + 1), nickLen);
			nick[nickLen] = '\0';
			offset += nickLen;
			uint16 typingType = (data[++offset] << 8) + data[++offset];
			
			LOG(kProtocolName, liLow, "Got typing notification (0x%04x) for "
				"\"%s\"", typingType, nick);
				
			fHandler->UserIsTyping(nick, (typing_notification)typingType);
			free(nick);
			
		} break;
		default: {
			LOG(kProtocolName, liMedium, "Got unhandled SNAC of family "
				"0x0004 (ICBM) of subtype 0x%04x", subtype);
			ret = kUnhandled;
		};
	};
	
	return ret;
};

status_t AIMManager::HandleBuddyList(BMessage *msg) {
	status_t ret = B_OK;
	ssize_t bytes = 0;
	const uchar *data = NULL;
	uint16 offset = 0;
	uint16 subtype = 0;
	uint16 flags = 0;

	msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
	subtype = (data[2] << 8) + data[3];
	flags = (data[4] << 8) + data[5];
	
	offset = 9;
	
	if (flags & 0x8000) {
		uint16 skip = (data[++offset] << 8) + data[++offset];
		offset += skip;
	};

	switch (subtype) {
		case USER_ONLINE: {	
//			This message contains lots of stuff, most of which we
//			ignore. We're good like that :)
			uint8 nickLen = data[++offset];
			char *nick = (char *)calloc(nickLen + 1, sizeof(char));
			memcpy(nick, (void *)(data + offset + 1), nickLen);
			nick[nickLen] = '\0';
		
			offset += nickLen;

//			These are currently unused.
			uint16 warningLevel = (data[++offset] << 8) + data[++offset];
			uint16 tlvs = (data[++offset] << 8) + data[++offset];

			while (offset < bytes) {
				uint16 tlvtype = (data[++offset] << 8) + data[++offset];
				uint16 tlvlen = (data[++offset] << 8) + data[++offset];
				switch (tlvtype) {
					case 0x0001: {	// User class / status
						uint16 userclass = (data[++offset] << 8) + data[++offset];

						if ((userclass & CLASS_AWAY) == CLASS_AWAY) {
							fHandler->StatusChanged(nick, OSCAR_AWAY);
						} else {
							fHandler->StatusChanged(nick, OSCAR_ONLINE);
						};
					} break;
					
					case 0x001d: {	// Icon / available message
						LOG(kProtocolName, liLow, "User %s has icon / available message.",
							nick);
						uint16 type;
						uint8 index;
						uint8 length;
						uint16 end = offset + tlvlen;

						while (offset < end) {
							type = (data[++offset] << 8) + data[++offset];
							index = data[++offset];
							length = data[++offset];
							switch (type) {
								case 0x0000: {	// Official icon
								} break;
								case 0x0001: {	// Icon
									char *v = (char *)calloc(length, sizeof(char));
									memcpy(v, data + offset + 1, length);

									Flap *buddy = new Flap(SNAC_DATA);
									buddy->AddSNAC(new SNAC(
										SERVER_STORED_BUDDY_ICONS, AIM_ICON_REQUEST));
									buddy->AddInt8(nickLen);
									buddy->AddRawData((uchar *)nick, nickLen);
									buddy->AddInt8(0x01);		// Command
									buddy->AddInt16(0x0001);	// Icon id
									buddy->AddInt8(0x01);		// Icon flags?
									buddy->AddInt8(0x10);		// Hash size
									buddy->AddRawData((uchar *)v, 0x10);
									Send(buddy);
									free(v);
								} break;
								case 0x0002: {
								} break;
								default: {
								};
							};

							offset += length;
						}; 
					} break;
					
					default: {
						offset += tlvlen;
					} break;
				}
			};

			free(nick);

		} break;
		case USER_OFFLINE: {
			offset = 10;
			uint8 nickLen = data[++offset];
			char *nick = (char *)calloc(nickLen + 1, sizeof(char));
			memcpy(nick, (void *)(data + offset), nickLen);
			nick[nickLen] = '\0';
								
			LOG(kProtocolName, liLow, "AIMManager: \"%s\" went offline", nick);
			
			fHandler->StatusChanged(nick, OSCAR_OFFLINE);
			free(nick);
		} break;
		default: {
			LOG(kProtocolName, liMedium, "Got an unhandled SNAC of family 0x0003 "
				"(Buddy List). Subtype 0x%04x", subtype);
			ret = kUnhandled;
		}
	};
	
	return ret;
};

status_t AIMManager::HandleSSI(BMessage *msg) {
	status_t ret = B_OK;
	ssize_t bytes = 0;
	const uchar *data = NULL;
	uint16 offset = 0;
	uint16 subtype = 0;
	uint16 flags = 0;

	msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
	subtype = (data[2] << 8) + data[3];
	flags = (data[4] << 8) + data[5];
	
	offset = 9;
	
	if (flags & 0x8000) {
		uint16 skip = (data[++offset] << 8) + data[++offset];
		offset += skip;
	};
	
	switch (subtype) {
		case SERVICE_PARAMETERS: {
			uint16 type = (data[++offset] << 8) + data[++offset];
			uint16 length = (data[++offset] << 8) + data[++offset];
			length;
				
			if (type == 0x0004) {
//				SSI Params
				for (int32 i = 0; i < kSSILimitCount; i++) {
					fSSILimits[i] = (data[++offset] << 8) + data[++offset];
				};
			};
		} break;
		case ROSTER_CHECKOUT: {
			list <BString> contacts;
		
			uint8 ssiVersion = data[++offset];
			uint16 itemCount = (data[++offset] << 8) + data[++offset];

			LOG(kProtocolName, liDebug, "SSI Version 0x%x", ssiVersion);
			LOG(kProtocolName, liLow, "%i SSI items", itemCount);

			fSSIItems = itemCount;

			for (uint16 i = 0; i < itemCount; i++) {
				
				uint16 nameLen = (data[++offset] << 8) + data[++offset];
				char *name = NULL;
				if (nameLen > 0) {
					name = (char *)calloc(nameLen + 1, sizeof(char));
					memcpy(name, (void *)(data + offset + 1), nameLen);
					name[nameLen] = '\0';

					offset += nameLen;
				};
							
				uint16 groupID = (data[++offset] << 8) + data[++offset];
				uint16 itemID = (data[++offset] << 8) + data[++offset];
				uint16 type = (data[++offset] << 8) + data[++offset];
				uint16 len = (data[++offset] << 8) + data[++offset];
				
				LOG(kProtocolName, liLow, "SSI item %i is of type 0x%04x (%i bytes)",
					 i, type, len);
				
				switch (type) {
					case GROUP_RECORD: {
						TLV tlv(data + offset, bytes - offset);
//						Bunch o' groups
						if (tlv.Type() == 0x00c8) {
							for (int32 i = 0; i < (tlv.Length() / 2); i++) {
								int16 buddyID = (data[offset + 4 + (i * 2)] << 8) +
									data[offset + 5 + (i * 2)];
							};
						};
					} break;
					case BUDDY_RECORD: {
//						There's some custom info here.
						fBuddy[name] = new Buddy(name, groupID, itemID);

						contacts.push_back(name);
					} break;
					case BUDDY_ICON_INFO: {
						const uchar *info = data + offset + 1;
						int16 position = 0;
						
						while (position < len) {
							TLV tlv(info + position, len - position);
							position += tlv.Length() + 4;
											
						};
					} break;
					default: {
					} break;
				};

				offset += len;
				if (name) free(name);
			};
						
			uint32 checkOut = (data[++offset] << 24) + (data[++offset] << 16) +
				(data[++offset] << 8) + data[++offset];	
			LOG(kProtocolName, liLow, "Last checkout of SSI list 0x%08x", checkOut);

			fHandler->SSIBuddies(contacts);
		} break;
		case SSI_MODIFY_ACK: {
			int16 count = 0;
			const char *error[] = {
				"Success",
				"N/A",
				"Item not found in list",
				"Item already exists",
				"N/A",
				"N/A",
				"N/A",
				"N/A",
				"N/A",
				"N/A",
				"Error adding - invalid ID / already in list / invalid data",
				"N/A",
				"Can't add item, limit exceeded",
				"ICQ cannot be added to AIM list",
				"Requires authorisation",
				"N/A"
			};
			
			while (offset < bytes) {
				uint16 code = (data[++offset] << 8) + data[++offset];
				LOG(kProtocolName, liHigh, "Upload for item %i is %s (0x%04x)",
					count, error[code], code);
				count++;
			};
		} break;
		default: {
			LOG(kProtocolName, liLow, "Got an unhandled SSI SNAC (0x0013 / 0x%04x)",
				subtype);
			ret = kUnhandled;
		} break;
	};
	
	return ret;
};

status_t AIMManager::HandleLocation(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleAdvertisement(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleInvitation(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleAdministrative(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandlePopupNotice(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandlePrivacy(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleUserLookup(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleUsageStats(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleTranslation(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleChatNavigation(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleChat(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleUserSearch(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleBuddyIcon(BMessage *msg) {
	status_t ret = B_OK;
	uint16 seqNum = msg->FindInt16("seqNum");
	uint16 flapLen = msg->FindInt16("flapLen");
	const uchar *data;
	int32 bytes = 0;
	msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);

	uint32 offset = 0;
	uint16 family = (data[offset] << 8) + data[++offset];
	uint16 subtype = (data[++offset] << 8) + data[++offset];
	uint16 flags = (data[++offset] << 8) + data[++offset];
	uint32 requestid = (data[++offset] << 24) + (data[++offset] << 16) +
		(data[++offset] << 8) + data[++offset];

	switch (subtype) {
		case AIM_ICON: {
			uint8 nickLen = data[++offset];
			char *nick = (char *)calloc(nickLen + 1, sizeof(char));
			uint16 iconType = 0;
			uint16 iconLength = 0;
			
			memcpy(nick, data + offset + 1, nickLen);
			offset += nickLen;
			iconType = (data[++offset] << 8) + data[++offset];
			offset += 18;	// Skip the hash and flags
			iconLength = (data[++offset] << 8) + data[++offset];

			if ((iconType == 0x0001) && (iconLength != 0)) {
				fHandler->BuddyIconFromUser(nick, data + offset + 1, iconLength);
			};
			free(nick);
		} break;
		
		case ICON_UPLOAD_ACK: {
		} break;
		
		default: {
			ret = kUnhandled;
		} break;
	};
	
	return ret;
};

status_t AIMManager::HandleICQ(BMessage *msg) {
	return kUnhandled;
};

status_t AIMManager::HandleAuthorisation(BMessage *msg) {
	return kUnhandled;
};

//#pragma mark -

status_t AIMManager::Send(Flap *f) {
	if (f == NULL) return B_ERROR;
	
	if (f->Channel() == SNAC_DATA) {
		SNAC *s = f->SNACAt(0);
		
		if (s != NULL) {
			uint16 family = s->Family();

			connlist::iterator i;
			
			for (i = fConnections.begin(); i != fConnections.end(); i++) {
				OSCARConnection *con = (*i);
				if (con == NULL) continue;
				if (con->Supports(family) == true) {
					LOG(kProtocolName, liLow, "Sending SNAC (0x%04x) via %s:%i", family,
						con->Server(), con->Port());
					con->Send(f);
					return B_OK;
				};
			}
					
			LOG(kProtocolName, liMedium, "No connections handle SNAC (0x%04x) requesting service",
				family);
			OSCARConnection *con = fConnections.front();
			if (con == NULL) {
				LOG(kProtocolName, liHigh, "No available connections to send SNAC");
				return B_ERROR;
			} else {
				pfc_map::iterator pIt = fPendingConnections.find(family);
				if (pIt == fPendingConnections.end()) {
					Flap *newService = new Flap(SNAC_DATA);
					newService->AddSNAC(new SNAC(SERVICE_CONTROL, REQUEST_NEW_SERVICE));
					newService->AddInt16(family);

					con->Send(newService, atImmediate);
					
					fPendingConnections[family] = NULL;
					fWaitingSupport.push_back(f);
				} else {
					if (pIt->second != NULL) {
						pIt->second->Send(f, atOnline);
					} else {
						fWaitingSupport.push_back(f);
					};
				};
			};
		};
	} else {
		OSCARConnection *con = fConnections.front();
		if (con != NULL) {
			con->Send(f);
		} else {
			return B_ERROR;
		};
	};
	
	return B_OK;
};

status_t AIMManager::Login(const char *server, uint16 port, const char *username,
	const char *password) {
	
	if ((username == NULL) || (password == NULL)) {
		LOG(kProtocolName, liHigh, "AIMManager::Login: username or password not set");
		return B_ERROR;
	}
	
	ClearConnections();
	ClearWaitingSupport();
	
	if (fConnectionState == OSCAR_OFFLINE) {
		uint8 nickLen = strlen(username);
	
		fOurNick = (char *)realloc(fOurNick, (nickLen + 1) * sizeof(char));
		strncpy(fOurNick, username, nickLen);
		fOurNick[nickLen] = '\0';

		fConnectionState = OSCAR_CONNECTING;

		Flap *flap = new Flap(OPEN_CONNECTION);

		flap->AddInt32(0x00000001);
		flap->AddTLV(0x0001, username, nickLen);

		char *encPass = EncodePassword(password);
		flap->AddTLV(0x0002, encPass, strlen(encPass));
		free(encPass);

		flap->AddTLV(0x0003, "BeOS IM Kit: AIM Addon", strlen("BeOS IM Kit: AIM Addon"));
		flap->AddTLV(0x0016, (char []){0x00, 0x01}, 2);
		flap->AddTLV(0x0017, (char []){0x00, 0x04}, 2);
		flap->AddTLV(0x0018, (char []){0x00, 0x02}, 2);
		flap->AddTLV(0x001a, (char []){0x00, 0x00}, 2);
		flap->AddTLV(0x000e, "us", 2);
		flap->AddTLV(0x000f, "en", 2);
		flap->AddTLV(0x0009, (char []){0x00, 0x15}, 2);
		
		OSCARConnection *c = new AIMBOSConnection(server, port, this);
		c->Run();
		fConnections.push_back(c);
		c->Send(flap);

		fHandler->Progress("AIM Login", "AIM: Connecting...", 0.10);

		return B_OK;
	} else {
		LOG(kProtocolName, liDebug, "AIMManager::Login: Already online");
		return B_ERROR;
	};
};

status_t AIMManager::Progress(const char *id, const char *msg, float progress) {
	return fHandler->Progress(id, msg, progress);
};

status_t AIMManager::Error(const char *msg) {
	return fHandler->Error(msg);
};

void AIMManager::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case AMAN_NEW_CAPABILITIES: {

			LOG(kProtocolName, liLow, "Got a possible new capability %i connections"
				", %i pending", fConnections.size(), fPendingConnections.size());

			int16 family = 0;
			pfc_map::iterator pIt;
			for (int32 i = 0; msg->FindInt16("family", i, &family) == B_OK; i++) {
				pIt = fPendingConnections.find(family);
				if (pIt != fPendingConnections.end()) {
					fPendingConnections.erase(pIt);
					OSCARConnection *c = pIt->second;
					if (c != NULL) {
						LOG(kProtocolName, liLow, "%s:%i (%s) handles a new "
							"capability 0x%04x", c->Server(), c->Port(),
							c->ConnName(), family);
						fConnections.push_back(c);
					} else {
						LOG(kProtocolName, liDebug, "Connection to support 0x%04x "
							"has gone null on our ass", family);
					};
				} else {
					LOG(kProtocolName, liMedium, "An unexpected connection came in "
						"for family 0x%04x", family);
				};
			};
					
//			We can cheat here. Just try resending all the items, Send() will
//			take care of finding a connection for it
			flap_stack::iterator i;
			for (i = fWaitingSupport.begin(); i != fWaitingSupport.end(); i++) {
				Flap *f = (*i);
				if (f) Send(f);
			};
			
			fWaitingSupport.clear();
		} break;
	
		case AMAN_STATUS_CHANGED: {
			uint8 status = msg->FindInt8("status");
			fHandler->StatusChanged(fOurNick, (online_types)status);
			fConnectionState = status;
		} break;
	
		case AMAN_NEW_CONNECTION: {
			const char *cookie;
			int32 bytes = 0;
			int16 port = 0;
			char *host = NULL;
			int16 family = -1;
			OSCARConnection *con = NULL;
			
			if (msg->FindData("cookie", B_RAW_TYPE, (const void **)&cookie, &bytes) != B_OK) return;
			if (msg->FindString("host", (const char **)&host) != B_OK) return;
			if (msg->FindInt16("port", &port) != B_OK) return;
			if (msg->FindInt16("family", &family) == B_OK) {
				LOG(kProtocolName, liMedium, "Connecting to %s:%i for 0x%04x\n",
					host, port, family);
				con = new AIMReqConn(host, port, this);
				fPendingConnections[family] = con;
			} else {
				con = new OSCARConnection(host, port, this);
				fConnections.push_back(con);
			};
			
			Flap *srvCookie = new Flap(OPEN_CONNECTION);
			srvCookie->AddRawData((uchar []){0x00, 0x00, 0x00, 0x01}, 4);
			srvCookie->AddTLV(new TLV(0x0006, cookie, bytes));

			con->Run();
			con->Send(srvCookie);
		} break;
		
		case AMAN_CLOSED_CONNECTION: {
			OSCARConnection *con = NULL;
			msg->FindPointer("connection", (void **)&con);
			if (con != NULL) {
				LOG(kProtocolName, liLow, "Connection (%s:%i) closed", con->Server(),
					con->Port());
					
				fConnections.remove(con);
				con->Lock();
				con->Quit();
				LOG(kProtocolName, liLow, "After close we have %i connections",
					fConnections.size());
				
				bool hasBOS = false;
				connlist::iterator cIt = fConnections.begin();
				for (; cIt != fConnections.end(); cIt++) {
					OSCARConnection *con = (*cIt);
					if ((con) && (con->ConnectionType() == connBOS)) {
						hasBOS = true;
						break;
					};
				};
				
				if (hasBOS == false) {
					ClearWaitingSupport();
					ClearConnections();
					fHandler->StatusChanged(fOurNick, OSCAR_OFFLINE);
					fConnectionState = OSCAR_OFFLINE;
				};
			};
		} break;

		case AMAN_FLAP_OPEN_CON: {
//			We don't do anything with this currently
//			const uchar *data;
//			int32 bytes = 0;
//			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
		} break;
		
		case AMAN_FLAP_SNAC_DATA: {
			const uchar *data;
			int32 bytes = 0;
			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
			
			uint32 offset = 0;
			uint16 family = (data[offset] << 8) + data[++offset];
			uint16 subtype = (data[++offset] << 8) + data[++offset];
			uint16 flags = (data[++offset] << 8) + data[++offset];
			uint32 requestid = (data[++offset] << 24) + (data[++offset] << 16) +
				(data[++offset] << 8) + data[++offset];

			LOG(kProtocolName, liLow, "AIMManager: Got SNAC (0x%04x, 0x%04x)", family, subtype);
			
			if (subtype == ERROR) {
				uint16 code = 0;
				code = (data[++offset] << 8) + data[++offset];
				fHandler->Error(kErrors[code]);
				return;
			};
			
			status_t result = kUnhandled;
			
			switch (family) {
				case SERVICE_CONTROL: {
					result = HandleServiceControl(msg);
				} break;
				case LOCATION: {
					result = HandleLocation(msg);
				} break;
				case BUDDY_LIST_MANAGEMENT: {
					result = HandleBuddyList(msg);
				} break;
				case ICBM: {
					result = HandleICBM(msg);
				} break;
				case ADVERTISEMENTS: {
					result = HandleAdvertisement(msg);
				} break;
				case INVITATION: {
					result = HandleInvitation(msg);
				} break;
				case ADMINISTRATIVE: {
					result = HandleAdministrative(msg);
				} break;
				case POPUP_NOTICES: {
					result = HandlePopupNotice(msg);
				} break;
				case PRIVACY_MANAGEMENT: {
					result = HandlePrivacy(msg);
				} break;
				case USER_LOOKUP: {
					result = HandleUserLookup(msg);
				} break;
				case USAGE_STATS: {
					result = HandleUsageStats(msg);
				} break;
				case TRANSLATION: {
					result = HandleTranslation(msg);
				} break;
				case CHAT_NAVIGATION: {
					result = HandleChatNavigation(msg);
				} break;
				case CHAT: {
					result = HandleChat(msg);
				} break;
				case DIRECTORY_USER_SEARCH: {
					result = HandleUserSearch(msg);
				} break;
				case SERVER_STORED_BUDDY_ICONS: {
					result = HandleBuddyIcon(msg);
				} break;
				case SERVER_SIDE_INFORMATION: {
					result = HandleSSI(msg);
				} break;
				case ICQ_SPECIFIC_EXTENSIONS: {
					result = HandleICQ(msg);
				} break;
				case AUTHORISATION_REGISTRATION: {
					result = HandleAuthorisation(msg);
				} break;
			};
			
			if (result == kUnhandled) {
				LOG(kProtocolName, liHigh, "Got totally unhandled SNAC (0x%04x"
					", 0x%04x)", family, subtype);
			};
		} break;
				
		case AMAN_FLAP_ERROR: {
//			We ignore this for now
		} break;
	
		case AMAN_FLAP_CLOSE_CON: {
			const uchar *data;
			int32 bytes = 0;
			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);

			if (fConnectionState == OSCAR_CONNECTING) {
	
				int32 i = 6;
				char *server = NULL;
				uint32 port = 0;
				char *cookie = NULL;
				uint16 cookieSize = 0;
	
				uint16 type = 0;
				uint16 length = 0;
				char *value = NULL;
	
				while (i < bytes) {
					type = (data[i] << 8) + data[++i];
					length = (data[++i] << 8) + data[++i];
					value = (char *)calloc(length + 1, sizeof(char));
					memcpy(value, (char *)(data + i + 1), length);
					value[length] = '\0';
	
					switch (type) {
						case 0x0001: {	// Our name, god knows why
						} break;
						
						case 0x0005: {	// New Server:IP
							char *colon = strchr(value, ':');
							port = atoi(colon + 1);
							server = (char *)calloc((colon - value) + 1,
								sizeof(char));
							strncpy(server, value, colon - value);
							server[(colon - value)] = '\0';
							
							LOG(kProtocolName, liHigh, "Need to reconnect to: %s:%i", server, port);
						} break;
	
						case 0x0006: {
							cookie = (char *)calloc(length, sizeof(char));
							memcpy(cookie, value, length);
							cookieSize = length;
						} break;
					};
	
					free(value);
					i += length + 1;
					
				};
				
				free(server);
				
				Flap *f = new Flap(OPEN_CONNECTION);
				f->AddInt32(0x00000001);
				f->AddTLV(0x0006, cookie, cookieSize);
				
				Send(f);
			} else {
				fHandler->StatusChanged(fOurNick, OSCAR_OFFLINE);
			};
		} break;
		
		default: {
			BLooper::MessageReceived(msg);
		};
	};
};

//#pragma mark Interface

status_t AIMManager::MessageUser(const char *screenname, const char *message) {
	LOG(kProtocolName, liLow, "AIMManager::MessageUser: Sending \"%s\" (%i) to %s (%i)",
		message, strlen(message), screenname, strlen(screenname));
		
	Flap *msg = new Flap(SNAC_DATA);
	msg->AddSNAC(new SNAC(ICBM, SEND_MESSAGE_VIA_SERVER));
	msg->AddInt64(0x0000000000000000); // MSG-ID Cookie
	msg->AddInt16(PLAIN_TEXT);	// Channel

	uint8 screenLen = strlen(screenname);
	msg->AddInt8(screenLen);
	msg->AddRawData((uchar *)screenname, screenLen);
	
	TLV *msgData = new TLV(0x0002);
	msgData->AddTLV(new TLV(0x0501, (char []){0x01}, 1));	// Text capability
	
	BString html = message;
	encode_html(html);
	char *msgFragment = (char *)calloc(html.Length() + 5, sizeof(char));
	msgFragment[0] = 0x00; // UTF-8. Maybe... who knows?
	msgFragment[1] = 0x00;
	msgFragment[2] = 0xff;
	msgFragment[3] = 0xff;
	strncpy(msgFragment + 4, html.String(), html.Length());
//	msgFragment[html.Length() + 4] = '\0';
	
	msgData->AddTLV(new TLV(0x0101, msgFragment, html.Length() + 4));
	
	free(msgFragment);
	msg->AddTLV(msgData);
	Send(msg);
	return B_OK;

//	Below kept for posterity.

/*	BString msg_encode(message);
	encode_html(msg_encode);
	
	int32 utf16_size = msg_encode.Length()*2+2;
	char * utf16_data = (char*)calloc( utf16_size, sizeof(char) );
	int32 utf8_size = msg_encode.Length();
	int32 state = 0;
	
	status_t res = convert_from_utf8(
		B_UNICODE_CONVERSION,
		msg_encode.String(),
		&utf8_size,
		utf16_data,
		&utf16_size,
		&state
	);
	
	if ( res != B_OK ) {
		LOG(kProtocolName, liDebug, "Conversion to UTF-16 failed" );
		return B_ERROR;
	}
	
	char *buffer = (char *)calloc(utf16_size + 4, sizeof(char));
	buffer[0] = 0x00;
	buffer[1] = 0x02; // utf-16be
	buffer[2] = 0xff;
	buffer[3] = 0xff;
	
	int16 utf16_size_be = htons(utf16_size + 0x0d);
*/
};

status_t AIMManager::AddBuddy(const char *buddy) {
	status_t ret = B_ERROR;
	if (buddy != NULL) {
		LOG(kProtocolName, liLow, "AIMManager::AddBuddy: Adding \"%s\" to list", buddy);
		fBuddy[buddy] = NULL;
		
		Flap *addBuddy = new Flap(SNAC_DATA);
		addBuddy->AddSNAC(new SNAC(BUDDY_LIST_MANAGEMENT, ADD_BUDDY_TO_LIST, 0x00,
			0x00, 0x00000000));
		
		uint8 buddyLen = strlen(buddy);
		addBuddy->AddRawData((uchar [])&buddyLen, sizeof(buddyLen));
		addBuddy->AddRawData((uchar *)buddy, buddyLen);
		
		ret = B_OK;
	};

	return ret;
};

status_t AIMManager::AddBuddies(list <char *>buddies) {
	Flap *addBuds = new Flap(SNAC_DATA);
	addBuds->AddSNAC(new SNAC(BUDDY_LIST_MANAGEMENT, ADD_BUDDY_TO_LIST, 0x00,
		0x00, 0x00000000));
	
	list <char *>::iterator i;
	uint8 buddyLen = 0;
	
	for (i = buddies.begin(); i != buddies.end(); i++) {
		char *buddy = (*i);
		buddyLen = strlen(buddy);

		fBuddy[buddy] = NULL;		
		
		addBuds->AddRawData((uchar *)&buddyLen, sizeof(buddyLen));
		addBuds->AddRawData((uchar *)buddy, buddyLen);
		
		free(buddy);
	};
	
	Send(addBuds);
	
	return B_OK;	
};

status_t AIMManager::RemoveBuddy(const char *buddy) {
	status_t ret = B_ERROR;

	if (buddy) {
		buddymap::iterator bIt = fBuddy.find(buddy);
		uint8 buddyLen = strlen(buddy);

		if ((bIt == fBuddy.end()) || (bIt->second == NULL)) {		
//			Not an SSI buddy, client side only
			Flap *remove = new Flap(SNAC_DATA);
			remove->AddSNAC(new SNAC(BUDDY_LIST_MANAGEMENT, REMOVE_BUDDY_FROM_LIST,
				0x00, 0x00, 0x00000000));
	
			remove->AddRawData((uchar *)&buddyLen, sizeof(buddyLen));
			remove->AddRawData((uchar *)buddy, buddyLen);
	
			Send(remove);
	
			ret = B_OK;
		} else {
//			Start modification session
			Flap *begin = new Flap(SNAC_DATA);
			begin->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_EDIT_BEGIN));
			Send(begin);

//			Buddy is in our SSI list
			Flap *remove = new Flap(SNAC_DATA);
			remove->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, DELETE_SSI_ITEM));
			
//			Buddy name
			remove->AddInt8(buddyLen);
			remove->AddRawData((uchar *)buddy, buddyLen);

			Buddy *ssi = bIt->second;
			uint16 groupID = ssi->GroupID();
			uint16 itemID = ssi->ItemID();
			uint16 type = BUDDY_RECORD;

			remove->AddInt16(groupID);
			remove->AddInt16(itemID);
			remove->AddInt16(BUDDY_RECORD);
			remove->AddInt16(0x0000); // No additional data

			Send(remove);
			
			fBuddy.erase(buddy);
			delete ssi;
			
			Flap *end = new Flap(SNAC_DATA);
			end->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_EDIT_END));
			Send(end);
			
			ret = B_OK;			
		};
	};
	
	return ret;
};

status_t AIMManager::RemoveBuddies(list <char *>buddy) {
	(void)buddy;
	return B_OK;
};

int32 AIMManager::Buddies(void) const {
	return fBuddy.size();
};

uchar AIMManager::IsConnected(void) const {
	return fConnectionState;
};

status_t AIMManager::LogOff(void) {
	status_t ret = B_ERROR;
	if (fConnectionState != OSCAR_OFFLINE) {
		fConnectionState = OSCAR_OFFLINE;
		
		LOG(kProtocolName, liLow, "%i connection(s) to kill", fConnections.size());
		
		ClearConnections();
		ClearWaitingSupport();

		fHandler->StatusChanged(fOurNick, OSCAR_OFFLINE);
		ret = B_OK;
	};

	return ret;
};

status_t AIMManager::TypingNotification(const char *buddy, uint16 typing) {
	LOG(kProtocolName, liLow, "Sending typing notification (0x%04x) to \"%s\"",
		typing, buddy);
	
	Flap *notify = new Flap(SNAC_DATA);
	notify->AddSNAC(new SNAC(ICBM, TYPING_NOTIFICATION));
	notify->AddInt64(0x0000000000000000);	// Notification cookie
	notify->AddInt16(PLAIN_TEXT);			// Notification channel

	uint8 buddyLen = strlen(buddy);
	notify->AddInt8(buddyLen);
	notify->AddRawData((uchar *)buddy, buddyLen);
	notify->AddInt16(typing);
	
	Send(notify);
	
	return B_OK;
};

status_t AIMManager::SetAway(const char *message) {
	Flap *away = new Flap(SNAC_DATA);
	away->AddSNAC(new SNAC(LOCATION, SET_USER_INFORMATION, 0x00, 0x00, 0x00000000));

	if (message) {
		away->AddTLV(new TLV(0x0003, kEncoding, strlen(kEncoding)));
		away->AddTLV(new TLV(0x0004, message, strlen(message)));
		fAwayMsg = message;

		fHandler->StatusChanged(fOurNick, OSCAR_AWAY);
		fConnectionState = OSCAR_AWAY;
	} else {
		away->AddTLV(new TLV(0x0003, kEncoding, strlen(kEncoding)));
		away->AddTLV(new TLV(0x0004, "", 0));
	
		fAwayMsg = "";
		
		fHandler->StatusChanged(fOurNick, OSCAR_ONLINE);
		fConnectionState = OSCAR_ONLINE;
	};
		
	Send(away);
	
	return B_OK;
};

status_t AIMManager::SetProfile(const char *profile) {
	if (profile == NULL) {
		fProfile = "";
	} else {
		fProfile.SetTo(profile);
	};

	if ((fConnectionState == OSCAR_ONLINE) || (fConnectionState == OSCAR_AWAY)) {
		Flap *p = new Flap(SNAC_DATA);
		p->AddSNAC(new SNAC(LOCATION, SET_USER_INFORMATION, 0x00, 0x00,
			0x00000000));
		
		if (fProfile.Length() > 0) {
			p->AddTLV(new TLV(0x0001, kEncoding, strlen(kEncoding)));
			p->AddTLV(new TLV(0x0002, fProfile.String(), fProfile.Length()));
		};
		
		if ((fConnectionState == OSCAR_AWAY) && (fAwayMsg.Length() > 0)) {
			p->AddTLV(new TLV(0x0003, kEncoding, strlen(kEncoding)));
			p->AddTLV(new TLV(0x0004, fAwayMsg.String(), fAwayMsg.Length()));
		};
				
		Send(p); 
	};
				
	return B_OK;
};

status_t AIMManager::SetIcon(const char *icon, int16 size) {
	if ((icon == NULL) || (size < 0)) return B_ERROR;

	if (fIcon) free(fIcon);
	fIcon = (char *)calloc(size, sizeof(char));
	fIconSize = size;
	memcpy(fIcon, icon, size);

	if (fConnectionState == OSCAR_OFFLINE) return B_ERROR;
	
	Flap *add = new Flap(SNAC_DATA);
	add->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, ADD_SSI_ITEM));
	add->AddInt16(0x0001);		// Size of name
	add->AddInt8('3');			// Name
	add->AddInt16(0x0000);		// Group ID
	add->AddInt16(0x1813);		// Item ID
	add->AddInt16(BUDDY_ICON_INFO);
	add->AddInt16(0x0016);		// Length of additional data
	
	char buffer[18];
	buffer[0] = 0x01; // Icon flags
	buffer[2] = 0x10; // MD5 Length
	MD5(icon, size, buffer + 2);
	add->AddTLV(new TLV(0x00d5, buffer, 18));
	add->AddTLV(new TLV(0x0131, "", 0));
	Send(add);
	
	return B_OK;
};

char *AIMManager::ParseMessage(TLV *parent) {
	int16 offset = 0;
	char *buffer = (char *)parent->Value();
	int16 bytes = parent->Length();
	char *message = NULL;
	int16 length = 0;
	 
	while (offset < bytes) {
		TLV tlv((uchar *)buffer + offset, bytes - offset);

		switch (tlv.Type()) {
			case 0x0101: {	// Message length, encoding and contents
				int16 subOffset = 0;
				char *value = (char *)tlv.Value();
				length = tlv.Length() - (sizeof(int16) * 2);
				// The charset and subset are part of the TLV				
				int16 charSet = (value[subOffset] << 8) + value[++subOffset];
				int16 charSubSet = (value[++subOffset] << 8) + value[++subOffset];
				subOffset += 1;

				message = (char *)calloc(length + 1, sizeof(char));
				memcpy(message, (char *)(value + subOffset), length);
				message[length] = '\0';

				// UTF-16 message
				if (charSet == 0x0002) {
					LOG(kProtocolName, liLow, "Got a UTF-16 encoded message");
					PrintHex((uchar *)message, length);
					char *msg16 = (char *)calloc(length, sizeof(char));
					int32 state = 0;
					int32 utf8Size = length * 2;
					int32 ut16Size = length;
					memcpy(msg16, message, length);
					message = (char *)realloc(message, utf8Size * sizeof(char));
					
					convert_to_utf8(B_UNICODE_CONVERSION, msg16, &ut16Size, message,
						&utf8Size, &state);
					message[utf8Size] = '\0';
					length = utf8Size;

					free(msg16);
					
					LOG(kProtocolName, liLow, "Converted message: \"%s\"",
						message);
				};
				
				return message;
			} break;
		
			default: {
				offset += tlv.FlattenedSize();
			};
		};
	};
	
	return message;
};

