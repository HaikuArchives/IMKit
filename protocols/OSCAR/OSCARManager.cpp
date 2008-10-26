#include "OSCARManager.h"

#include <libim/Protocol.h>
#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <UTF8.h>
#include <ctype.h>
#include <string.h>

#include <openssl/md5.h>

#include "FLAP.h"
#include "TLV.h"
#include "Buddy.h"
#include "OSCARConnection.h"
#include "OSCARBOSConnection.h"
#include "OSCARReqConn.h"
#include "OSCARHandler.h"
#include "htmlparse.h"
#include "Group.h"
#include "BufferReader.h"
#include "BufferWriter.h"

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

const uint16 OSCAR_ERROR_COUNT = 0x18;
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

const uint16 kSSIResultCount = 14;
const char *kSSIResult[] = {
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
	"ICQ contacts cannot be added to AIM list",
	"Requires authorisation"
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
				if ((buf[j] < 0x21) || (buf[j] > 0x7d)) {
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

OSCARManager::OSCARManager(OSCARHandler *handler, const char *protocol) {	
	fConnectionState = OSCAR_OFFLINE;

	// Setup the map between SNAC families and the handling function
	fSNACHandler[SERVICE_CONTROL] = &OSCARManager::HandleServiceControl;
	fSNACHandler[LOCATION] = &OSCARManager::HandleLocation;
	fSNACHandler[BUDDY_LIST_MANAGEMENT] = &OSCARManager::HandleBuddyList;
	fSNACHandler[ICBM] = &OSCARManager::HandleICBM;
	fSNACHandler[ADVERTISEMENTS] = &OSCARManager::HandleAdvertisement;
	fSNACHandler[INVITATION] = &OSCARManager::HandleInvitation;
	fSNACHandler[ADMINISTRATIVE] = &OSCARManager::HandleAdministrative;
	fSNACHandler[POPUP_NOTICES] = &OSCARManager::HandlePopupNotice;
	fSNACHandler[PRIVACY_MANAGEMENT] = &OSCARManager::HandlePrivacy;
	fSNACHandler[USER_LOOKUP] = &OSCARManager::HandleUserLookup;
	fSNACHandler[USAGE_STATS] = &OSCARManager::HandleUsageStats;
	fSNACHandler[TRANSLATION] = &OSCARManager::HandleTranslation;
	fSNACHandler[CHAT_NAVIGATION] = &OSCARManager::HandleChatNavigation;
	fSNACHandler[CHAT] = &OSCARManager::HandleChat;
	fSNACHandler[DIRECTORY_USER_SEARCH] = &OSCARManager::HandleUserSearch;
	fSNACHandler[SERVER_STORED_BUDDY_ICONS] = &OSCARManager::HandleBuddyIcon;
	fSNACHandler[SERVER_SIDE_INFORMATION] = &OSCARManager::HandleSSI;
	fSNACHandler[ICQ_SPECIFIC_EXTENSIONS] = &OSCARManager::HandleICQ;
	fSNACHandler[AUTHORISATION_REGISTRATION] = &OSCARManager::HandleAuthorisation;

	fProtocol = protocol;
	fHandler = handler;
	fOurNick = NULL;
	fProfile.SetTo("IMKit OSCAR");
	
	fIcon = NULL;
	fIconSize = -1;
};

OSCARManager::~OSCARManager(void) {
	free(fOurNick);
	if (fIcon) free(fIcon);

	LogOff();
}

//#pragma mark -

const char *OSCARManager::Protocol(void) {
	return fProtocol.String();
};

status_t OSCARManager::ClearConnections(void) {
	LOG(Protocol(), liLow, "%i pending connections to close",
		fPendingConnections.size());

	pfc_map::iterator pIt;
	for (pIt = fPendingConnections.begin(); pIt != fPendingConnections.end(); pIt++) {
		OSCARReqConn *con = dynamic_cast<OSCARReqConn *>(pIt->second);
		if (con == NULL) continue;
		BMessenger(con).SendMessage(B_QUIT_REQUESTED);
	};
	fPendingConnections.clear();


	LOG(Protocol(), liLow, "%i used connections to close", fConnections.size());

	connlist::iterator it;
	for (it = fConnections.begin(); it != fConnections.end(); it++) {
		OSCARConnection *con = (*it);
		if (con == NULL) continue;

		BMessenger(con).SendMessage(B_QUIT_REQUESTED);
	};	
	fConnections.clear();
	
	return B_OK;
};

status_t OSCARManager::ClearWaitingSupport(void) {
	flap_stack::iterator it;
	
	for (it = fWaitingSupport.begin(); it != fWaitingSupport.end(); it++) {
		Flap *f = (*it);
		delete f;
	};
	
	fWaitingSupport.clear();
	
	return B_OK;
};

status_t OSCARManager::HandleServiceControl(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;
	
	int16 subtype = snac->SubType();
	int32 request = snac->RequestID();

	reader->OffsetTo(snac->DataOffset());
	
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
			LOG(Protocol(), liHigh, "Got extended status request");
			reader->Debug();
		
#if 0
			while (reader->HasMoreData()) {
				int16 notice = reader->ReadInt16();

				switch (notice) {
//					Official icon
					case 0x0000: {
						int16 length = reader->ReadInt16();
						reader->OffsetBy(length);
					} break;
					case 0x0001: {
						LOG(Protocol(), liHigh, "Got icon type 0x0001 - sending");
					
						uint8 flags = reader->ReadInt8();
						int8 hashLen = reader->ReadInt8();
						uchar *currentHash = reader->ReadData(hashLen);
						
						if (flags == 0x41) {
						
//						if ((fIcon) && (fIconSize > 0)) {
//							uchar hash[hashLen];
//							MD5((uchar *)fIcon, fIconSize, (uchar *)hash);
							
//							if (memcmp(hash, currentHash, hashLen) != 0) {
//								LOG(Protocol(), liHigh, "Server stored buddy "
//									"icon is different to ours - uploading");
								Flap *upload = new Flap(SNAC_DATA);
								upload->AddSNAC(new SNAC(SERVER_STORED_BUDDY_ICONS,
									 ICON_UPLOAD_REQ));
//								upload->AddInt16(fSSIItems++);	// Next SSI item
								upload->AddInt16(0x0001);
								upload->AddInt16(fIconSize);
								upload->AddRawData((uchar *)fIcon, fIconSize);
								
								Send(upload);
//							};
						};
						
						free(currentHash);
					} break;
				};
			};
#endif			
		} break;
		
		case MOTD: {
			PrintHex(reader->Buffer(), reader->Length(), true);
			ret = B_OK;
		} break;
		
		case RATE_LIMIT_WARNING: {
			LOG(Protocol(), liHigh, "Rate limit warning!");
		} break;
		
		default: {
			ret = kUnhandled;
		};
	};

	return ret;
};

status_t OSCARManager::HandleICBM(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;
	
	uint16 subtype = snac->SubType();
	uint32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());
	
	switch (subtype) {
		case ERROR: {
			LOG(Protocol(), liHigh, "Server error 0x%04x", reader->ReadInt16());
		} break;
		
		case MESSAGE_FROM_SERVER: {
			int64 messageId = reader->ReadInt64();
			uint16 channel = reader->ReadInt16();
			int8 nickLen = reader->ReadInt8();
			char *nick = reader->ReadString(nickLen);
			uint16 warning = reader->ReadInt16();
			uint16 tlvCount = reader->ReadInt16();
			bool autoReply = false;
			int16 userStatus = 0x0000;
			int16 userClass = 0x0000;
			char *message = NULL;

			for (int32 i = 0; i < tlvCount; i++) {
				TLV tlv(reader);
				BufferReader *tlvReader = tlv.Reader();
				
				switch (tlv.Type()) {
					case 0x0001: {	// Userclass
						userClass = tlvReader->ReadInt16();
						LOG(Protocol(), liHigh, "Got user class: 0x%04x", userClass);
					} break;
					case 0x0004: {	// Automated reply
						autoReply = true;
					} break;
					case 0x0006: {	// User status
						userStatus = tlvReader->ReadInt16();
					} break;
				};
				
				delete tlvReader;
			};
					
			switch (channel) {
				case PLAIN_TEXT: {
					// There should only be one TLV here, but who knows
					while (reader->HasMoreData()) {
						TLV tlv(reader);
						BufferReader *tlvReader = tlv.Reader();
	
						if (tlv.Type() == 0x0002) {
							while (tlvReader->Offset() < tlvReader->Length()) {
								int8 id = tlvReader->ReadInt8();
								int8 version = tlvReader->ReadInt8();
								int16 length = tlvReader->ReadInt16();
													
								// If it's not the text fragment, just skip it
								if (id == 0x01) {
									uint16 charSet = tlvReader->ReadInt16();
									uint16 charSubset = tlvReader->ReadInt16();
									uint16 messageLen = length - (sizeof(int16) * 2);
									message = tlvReader->ReadString(messageLen);
									
									if (charSet == 0x0002){
										LOG(Protocol(), liLow, "Got a UTF-16 encoded message");
										char *msg16 = (char *)calloc(messageLen,
											sizeof(char));
										int32 state = 0;
										int32 utf8Size = length * 2;
										int32 ut16Size = length;
										memcpy(msg16, message, length);
										message = (char *)realloc(message, utf8Size *
											sizeof(char));
										
										convert_to_utf8(B_UNICODE_CONVERSION, msg16,
											&ut16Size, message, &utf8Size, &state);
										message[utf8Size] = '\0';
										length = utf8Size;
					
										free(msg16);
						
										LOG(Protocol(), liLow, "Converted message: \"%s\"",
											message);
									};
								} else {
									tlvReader->OffsetBy(length);
								};
							};
						};
	
						delete tlvReader;
					};
				} break;
				
				case TYPED_OLD_STYLE: {
					while (reader->HasMoreData()) {
						TLV tlv(reader);
						BufferReader *tlvReader = tlv.Reader(B_SWAP_LENDIAN_TO_HOST);
						
						if (tlv.Type() == 0x0005) {	// Message data
							uint32 uin = tlvReader->ReadInt32();
							uint8 type = tlvReader->ReadInt8();
							uint8 flags = tlvReader->ReadInt8();
							uint16 length = tlvReader->ReadInt16();
							
							autoReply = (flags & MESSAGE_FLAG_AUTO_RESPONSE);
							
							switch (type) {
								case MESSAGE_TYPE_PLAIN: {
									message = tlvReader->ReadString(length);
								} break;

								default: {
									LOG(Protocol(), liHigh, "Got an unknown message "
										"type: 0x%02x", type);
									reader->Debug();
								};
							};
						};
					};
				} break;
				
				default: {
					BString error;
					error << nick;
					error << " sent you a message in an unsupported format (";
					error << channel << ") Please tell them to stop being an "
						"asshat. Additionally you may wish to seek out help on"
						" irc.freenode.net #beosimkit";
					fHandler->Error(error.String());
					
					LOG(Protocol(), liHigh, "Message on non-supported channel! "
						"(0x%04x)!", channel);
					reader->Debug();
				};
			};
			
			if (message) {
				parse_html(message);
										
				// If the user is invisible, get their real status and update
				if ((userStatus & STATUS_INVISIBLE) == STATUS_INVISIBLE) {
					uint16 statusMask = userStatus & 0x00ff;
					online_types status = OSCAR_AWAY;
					
					if (statusMask & STATUS_FREE_FOR_CHAT) status = OSCAR_ONLINE;
					if (statusMask == STATUS_ONLINE) status = OSCAR_ONLINE;
					
					fHandler->StatusChanged(nick, status);
				};

				fHandler->MessageFromUser(nick, message, autoReply);
			};

			free(nick);
			free(message);
		} break;
		
		case TYPING_NOTIFICATION: {
			uint64 id = reader->ReadInt64();
			uint16 channel = reader->ReadInt16();
			uint8 nickLen = reader->ReadInt8();
			char *nick = reader->ReadString(nickLen);
			uint16 type = reader->ReadInt16();
			
			LOG(Protocol(), liDebug, "Got typing notification (0x%04x) for "
				"\"%s\"", type, nick);
			
			fHandler->UserIsTyping(nick, (typing_notification)type);
			free(nick);
			
		} break;
		
		case SERVER_MISSED_MESSAGE: {
			LOG(Protocol(), liHigh, "Got a server missed message!");
			PrintHex(reader->Buffer(), reader->Length(), true);
		} break;
		
		default: {
			LOG(Protocol(), liMedium, "Got unhandled SNAC of family 0x0004 "
				"(ICBM) of subtype 0x%04x", subtype);
			ret = kUnhandled;
		};
	};
	
	return ret;
};

status_t OSCARManager::HandleBuddyList(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;
	
	int16 subtype = snac->SubType();
	int32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());

	switch (subtype) {
		case USER_ONLINE: {
		
		
			// There can be multiple users in a packet
			while (reader->Offset() < reader->Length()) {
				//	This message contains lots of stuff, most of which we
				//	ignore. We're good like that :)
				uint8 nickLen = reader->ReadInt8();
				char *nick = reader->ReadString(nickLen);
				uint16 userclass = 0;
				Buddy *buddy = NULL;
				buddymap::iterator bIt = fBuddy.find(nick);
				
				if (bIt == fBuddy.end()) {
					buddy = new Buddy(nick, -1);
					fBuddy[nick] = buddy;
				} else {
					buddy = bIt->second;
				};
				
				//	These are currently unused.
				uint16 warningLevel = reader->ReadInt16();
				uint16 tlvs = reader->ReadInt16();
	
				for (int32 i = 0; i < tlvs; i++) {
					TLV tlv(reader);
					BufferReader *tlvReader = tlv.Reader();
					
					switch (tlv.Type()) {
						case 0x0001: {	// User class / status
							userclass = tlvReader->ReadInt16();
							
							printf("User Mc Classian: 0x%04x\n", userclass);
						} break;
						
						case 0x001d: {	// Icon / available message
							LOG(Protocol(), liLow, "User %s has an icon / available message",
								nick);
							uint16 type = tlvReader->ReadInt16();
							uint8 flags = tlvReader->ReadInt8();
							uint8 hashLen = tlvReader->ReadInt8();
							uchar *hash = tlvReader->ReadData(hashLen);
							
							
							Flap *icon = new Flap(SNAC_DATA);
							icon->AddSNAC(new SNAC(SERVER_STORED_BUDDY_ICONS,
								AIM_ICON_REQUEST));
							icon->AddInt8(nickLen);
							icon->AddRawData((uchar *)nick, nickLen);
							icon->AddInt8(0x01);	// Command
							icon->AddInt16(type);	// Icon Id
							icon->AddInt8(flags);
							icon->AddInt8(hashLen);
							icon->AddRawData(hash, hashLen);
							
							Send(icon);
							
							free(hash);
						} break;
						
						case 0x000d: {
							while (tlvReader->HasMoreData()) {
								int32 caplen = 16;
								char *cap = (char *)tlvReader->ReadData(caplen);
								
								printf("We don't need no hats!\n");
								PrintHex((uchar *)cap, caplen, true);
								
								if (buddy->HasCapability(cap, caplen) == false) {
									buddy->AddCapability(cap, caplen);
								};
								
								free(cap);
							};
						} break;
					};
					
					delete tlvReader;
				};			
				
				buddy->SetUserclass(userclass);
				
				if ((userclass & CLASS_AWAY) == CLASS_AWAY) {
					fHandler->StatusChanged(nick, OSCAR_AWAY, buddy->IsMobileUser());
				} else {
					fHandler->StatusChanged(nick, OSCAR_ONLINE, buddy->IsMobileUser());
				};
			
				free(nick);
			}
		} break;
		case USER_OFFLINE: {
			uint8 nickLen = reader->ReadInt8();
			char *nick = reader->ReadString(nickLen);
			buddymap::iterator bIt = fBuddy.find(nick);
			if (bIt != fBuddy.end()) bIt->second->ClearCapabilities();
								
			LOG(Protocol(), liLow, "OSCARManager: \"%s\" went offline", nick);
			
			fHandler->StatusChanged(nick, OSCAR_OFFLINE);
			free(nick);
		} break;
		default: {
			LOG(Protocol(), liMedium, "Got an unhandled SNAC of family 0x0003 "
				"(Buddy List). Subtype 0x%04x", subtype);
			ret = kUnhandled;
		}
	};
	
	return ret;
};

status_t OSCARManager::HandleSSI(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;

	int16 subtype = snac->SubType();
	int32 request = snac->RequestID();

	reader->OffsetTo(snac->DataOffset());
	
	switch (subtype) {
		case SERVICE_PARAMETERS: {
			TLV limits(reader);
			BufferReader *tlvReader = limits.Reader();
				
			if (limits.Type() == 0x0004) {
				// SSI Params
				for (int32 i = 0; i < kSSILimitCount; i++) {
					fSSILimits[i] = tlvReader->ReadInt16();
					LOG(Protocol(), liHigh, "SSI Limit: %i: %i", i, fSSILimits[i]);
				};
			};
			
			delete tlvReader;
		} break;
		case ROSTER_CHECKOUT: {	
			list <BString> contacts;
		
			uint8 ssiVersion = reader->ReadInt8();
			uint16 itemCount = reader->ReadInt16();

			LOG(Protocol(), liDebug, "SSI Version 0x%x", ssiVersion);
			LOG(Protocol(), liLow, "%i SSI items", itemCount);

			fSSIItems = itemCount;

			for (uint16 i = 0; i < itemCount; i++) {
				LOG(Protocol(), liDebug, "Item %i / %i", i, itemCount);
				
				uint16 nameLen = reader->ReadInt16();
				char *name = reader->ReadString(nameLen);
							
				uint16 groupID = reader->ReadInt16();
				uint16 itemID = reader->ReadInt16();
				uint16 type = reader->ReadInt16();
				uint16 len = reader->ReadInt16();
				
				fItemIds[itemID] = true;
				
				LOG(Protocol(), liLow, "SSI item %i is of type 0x%04x (%i bytes)",
					 i, type, len);
				
				switch (type) {
					case GROUP_RECORD: {
						int32 end = reader->Offset() + len;
						
						while (reader->Offset() < end) {
							TLV tlv(reader);
							int16 size = tlv.Length();
							BufferReader *tlvReader = tlv.Reader();
						
							Group *group = new Group(groupID, name);
						
							LOG(Protocol(), liLow, "Group %s (0x%04x)", name, groupID);
	
							// Bunch o' groups
							if (tlv.Type() == 0x00c8) {
								for (int32 i = 0; i < (size / 2); i++) {
									int16 child = tlvReader->ReadInt16();
									LOG(Protocol(), liDebug, "\tChild 0x%04x", child);
									group->AddItem(child);
								};
							};
	
							delete tlvReader;
							fGroups[groupID] = group;
						};
					} break;
					case BUDDY_RECORD: {
						//	There's some custom info here.
						buddymap::iterator bIt = fBuddy.find(name);
						Buddy *buddy = NULL;
						if (bIt == fBuddy.end()) {
							buddy = new Buddy(name, itemID);
							fBuddy[name] = buddy;
						} else {
							buddy = bIt->second;
						};
						
						if (buddy->IsInGroup(groupID) == false) buddy->AddGroup(groupID);
						contacts.push_back(name);

						reader->OffsetBy(len);
						
						LOG(Protocol(), liDebug, "Got contact %s (0x%04x)", name, itemID);
					} break;
					case BUDDY_ICON_INFO: {
						reader->OffsetBy(len);
					} break;
					default: {
						reader->OffsetBy(len);
					} break;
				};

				if (name) free(name);
			};
						
			uint32 checkOut = reader->ReadInt32();
			LOG(Protocol(), liLow, "Last checkout of SSI list 0x%08x", checkOut);

			fHandler->SSIBuddies(contacts);
		} break;
		
		case SSI_MODIFY_ACK: {
			int16 count = 0;
			
			while (reader->Offset() < reader->Length()) {
				uint16 code = reader->ReadInt16();
				LOG(Protocol(), liMedium, "Upload for item %i is %s (0x%04x)",
					count, kSSIResult[code], code);
				count++;
			};
		} break;

		case AUTHORISATION_REQUEST: {
			uint8 idLength = reader->ReadInt8();
			char *id = reader->ReadString(idLength);
			uint16 reasonLength = reader->ReadInt16();
			char *reason = reader->ReadString(reasonLength);
		} break;

		default: {
			LOG(Protocol(), liHigh, "Got an unhandled SSI SNAC (0x0013 / 0x%04x)",
				subtype);
			reader->Debug();
			ret = kUnhandled;
		} break;
	};
	
	return ret;
};

status_t OSCARManager::HandleLocation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleAdvertisement(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleInvitation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleAdministrative(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandlePopupNotice(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandlePrivacy(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleUserLookup(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleUsageStats(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleTranslation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleChatNavigation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleChat(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleUserSearch(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARManager::HandleBuddyIcon(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;
	
	uint16 subtype = snac->SubType();
	uint32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());

	switch (subtype) {
		case AIM_ICON: {
			uint8 nickLen = reader->ReadInt8();
			char *nick = reader->ReadString(nickLen);
			uint16 type = reader->ReadInt16();
			uint8 flags = reader->ReadInt8();
			uint8 hashLen = reader->ReadInt8();
			uchar *hash = reader->ReadData(hashLen);
			uint16 iconLen = reader->ReadInt16();
			uchar *icon = reader->ReadData(iconLen);
			
			if ((type == 0x0001) && (iconLen > 0)) {
				fHandler->BuddyIconFromUser(nick, icon, iconLen);
			};
			
			free(nick);
			free(hash);
		} break;
		
		case ICON_UPLOAD_ACK: {
			LOG(Protocol(), liHigh, "Got ICON_UPLOAD_ACK");
			reader->Debug();
		} break;
		
		default: {
			ret = kUnhandled;
		} break;
	};
	
	return ret;
};

status_t OSCARManager::HandleICQ(SNAC *snac, BufferReader *reader) {
	status_t ret = kUnhandled;
	
	uint16 subtype = snac->SubType();
	uint32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());
	
	switch (subtype) {
		case META_INFORMATION_RESPONSE: {
			while (reader->HasMoreData()) {
				TLV tlv(reader);
				BufferReader *tlvReader = tlv.Reader(B_SWAP_LENDIAN_TO_HOST);
			
				switch (tlv.Type()) {
					case 0x0001: {	// Encapsulated meta data
						uint16 chunkSize = tlvReader->ReadInt16();
						uint32 targetUIN = tlvReader->ReadInt32();
						uint16 dataType = tlvReader->ReadInt16();
						uint16 sequence = tlvReader->ReadInt16();

						switch (dataType) {
							case 0x0041: {				// Offline message
								ret = B_OK;				// Handled!

								uint32 uin = tlvReader->ReadInt32();
															
 								uint16 year = tlvReader->ReadInt16();
								uint8 month = tlvReader->ReadInt8();
								uint8 day = tlvReader->ReadInt8();
								uint8 hour = tlvReader->ReadInt8();
								uint8 minute = tlvReader->ReadInt8();
								uint8 type = tlvReader->ReadInt8();
								uint8 flags = tlvReader->ReadInt8();
								
								uint16 length = tlvReader->ReadInt16();
								char *message = tlvReader->ReadString(length);
								
								char id[256];
								snprintf(id, sizeof(id), "%i", uin);
								
								fHandler->MessageFromUser(id, message,
									(flags & 0x03) == 0x03);
							} break;

							case 0x0042: {				// End of Offline messages
								ret = B_OK;
								
								// Tell the server to delete the messages
								Flap *delMsgs = new Flap(SNAC_DATA);
								delMsgs->AddSNAC(new SNAC(ICQ_SPECIFIC_EXTENSIONS,
									META_INFORMATION_REQUEST));
								
								// ICQ TLVs are Lendian
								BufferWriter writer(B_SWAP_HOST_TO_LENDIAN);
								writer.WriteInt16(0x0008);	// Always 8 bytes long
								
								int32 uin = strtol(fOurNick, NULL, 10);
								writer.WriteInt32(uin);
								writer.WriteInt16(0x003e);	// Delete meta command
								writer.WriteInt16(0x0000);
								
								TLV *tlv = new TLV(0x0001);	// Encapsulated meta
								tlv->Value((const char *)writer.Buffer(),
									writer.Length());
								delMsgs->AddTLV(tlv);
								
								Send(delMsgs);
							} break;
						};
					};
				};
				
				delete tlvReader;	
			};
		} break;
	};
	
	return ret;
};

status_t OSCARManager::HandleAuthorisation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

//#pragma mark -

status_t OSCARManager::Send(Flap *f) {
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
					LOG(Protocol(), liLow, "Sending SNAC (0x%04x) via %s:%i", family,
						con->Server(), con->Port());
					con->Send(f);
					return B_OK;
				};
			}
					
			LOG(Protocol(), liMedium, "No connections handle SNAC (0x%04x) requesting service",
				family);
			OSCARConnection *con = fConnections.front();
			if (con == NULL) {
				LOG(Protocol(), liHigh, "No available connections to send SNAC");
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

status_t OSCARManager::Login(const char *server, uint16 port, const char *username,
	const char *password) {
	
	if ((username == NULL) || (password == NULL)) {
		LOG(Protocol(), liHigh, "OSCARManager::Login: username or password not set");
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

		char *encPass = fHandler->RoastPassword(password);
		flap->AddTLV(0x0002, encPass, strlen(encPass));
		free(encPass);

		flap->AddTLV(0x0003, "BeOS IM Kit: OSCAR Addon", strlen("BeOS IM Kit: OSCAR Addon"));
		flap->AddTLV(0x0016, (char []){0x00, 0x01}, 2);
		flap->AddTLV(0x0017, (char []){0x00, 0x04}, 2);
		flap->AddTLV(0x0018, (char []){0x00, 0x02}, 2);
		flap->AddTLV(0x001a, (char []){0x00, 0x00}, 2);
		flap->AddTLV(0x000e, "us", 2);
		flap->AddTLV(0x000f, "en", 2);
		flap->AddTLV(0x0009, (char []){0x00, 0x15}, 2);
		
		OSCARConnection *c = new OSCARBOSConnection(server, port, this);
		c->Run();
		fConnections.push_back(c);
		c->Send(flap);

		fHandler->Progress("OSCAR Login", "OSCAR: Connecting...", 0.10);

		return B_OK;
	} else {
		LOG(Protocol(), liDebug, "OSCARManager::Login: Already online");
		return B_ERROR;
	};
};

status_t OSCARManager::Progress(const char *id, const char *msg, float progress) {
	return fHandler->Progress(id, msg, progress);
};

status_t OSCARManager::Error(const char *msg) {
	return fHandler->Error(msg);
};

void OSCARManager::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case AMAN_NEW_CAPABILITIES: {

			LOG(Protocol(), liLow, "Got a possible new capability %i connections"
				", %i pending", fConnections.size(), fPendingConnections.size());

			int16 family = 0;
			pfc_map::iterator pIt;
			for (int32 i = 0; msg->FindInt16("family", i, &family) == B_OK; i++) {
				pIt = fPendingConnections.find(family);
				if (pIt != fPendingConnections.end()) {
					fPendingConnections.erase(pIt);
					OSCARConnection *c = pIt->second;
					if (c != NULL) {
						LOG(Protocol(), liLow, "%s:%i (%s) handles a new "
							"capability 0x%04x", c->Server(), c->Port(),
							c->ConnName(), family);
						fConnections.push_back(c);
					} else {
						LOG(Protocol(), liDebug, "Connection to support 0x%04x "
							"has gone null on our ass", family);
					};
				} else {
					LOG(Protocol(), liMedium, "An unexpected connection came in "
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
			
			if ((fProtocol== "icq") && (status == OSCAR_ONLINE) || (status == OSCAR_AWAY)) {
				// Request offline messages
				Flap *offline = new Flap(SNAC_DATA);
				offline->AddSNAC(new SNAC(ICQ_SPECIFIC_EXTENSIONS, META_INFORMATION_REQUEST));
			

				// The content of the TLV are Lendian, for some reason
				BufferWriter writer(B_SWAP_HOST_TO_LENDIAN);
				writer.WriteInt16(0x0008);	// Length of internal data, always 8
				int32 uin = strtol(fOurNick, NULL, 10);
				writer.WriteInt32(uin);
				writer.WriteInt16(0x003c);	// Offline request
				writer.WriteInt16(0x0000);	// Request

				TLV *tlv = new TLV(0x0001);	// Encapsulated meta data
				tlv->Value((const char *)writer.Buffer(), writer.Length());
				
				offline->AddTLV(tlv);
				
				Send(offline);
			};
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
				LOG(Protocol(), liMedium, "Connecting to %s:%i for 0x%04x\n",
					host, port, family);
				con = new OSCARReqConn(host, port, this);
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
				LOG(Protocol(), liLow, "Connection (%s:%i) closed", con->Server(),
					con->Port());
					
				fConnections.remove(con);
				con->Lock();
				con->Quit();
				LOG(Protocol(), liLow, "After close we have %i connections",
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
			status_t result = kUnhandled;
			const uchar *data;
			int32 bytes = 0;
			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
			
			BufferReader reader(data, bytes);
			SNAC snac(&reader);		
			reader.OffsetTo(0);

			uint16 family = snac.Family();
			uint16 subtype = snac.SubType();

			LOG(Protocol(), liLow, "OSCARManager: Got SNAC (0x%04x, 0x%04x)", family, subtype);

			if (subtype == ERROR) {
				reader.OffsetTo(snac.DataOffset());
				uint16 code = reader.ReadInt16();
				fHandler->Error(kErrors[code]);
				return;
			};
			
			manhandler_t::iterator hIt = fSNACHandler.find(family);
			if (hIt != fSNACHandler.end()) {
				FamilyManHandler handler = hIt->second;
				result = (this->*handler)(&snac, &reader);
			} else {
				LOG(Protocol(), liHigh, "OSCARManager: Got a SNAC (0x%04x, 0x%04x) that "
					"had no handler\n", family, subtype);
			};
			if (result == kUnhandled) {
				LOG(Protocol(), liHigh, "Got totally unhandled SNAC (0x%04x, 0x%04x)", family,
					subtype);
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
							
							LOG(Protocol(), liHigh, "Need to reconnect to: %s:%i", server, port);
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

status_t OSCARManager::MessageUser(const char *screenname, const char *message) {
	LOG(Protocol(), liLow, "OSCARManager::MessageUser: Sending \"%s\" (%i) to %s (%i)",
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
	fHandler->FormatMessageText(html);
	char *msgFragment = (char *)calloc(html.Length() + 5, sizeof(char));
	msgFragment[0] = 0x00; // UTF-8. Maybe... who knows?
	msgFragment[1] = 0x00;
	msgFragment[2] = 0xff;
	msgFragment[3] = 0xff;
	strncpy(msgFragment + 4, html.String(), html.Length());
	
	msgData->AddTLV(new TLV(0x0101, msgFragment, html.Length() + 4));
	free(msgFragment);
	msg->AddTLV(msgData);
	msg->AddTLV(new TLV(0x0006, "", strlen("")));
	Send(msg);
	
	return B_OK;
};

status_t OSCARManager::AddSSIBuddy(const char *name, grouplist_t groups) {
	LOG(Protocol(), liLow, "OSCARManager::AddSSIBuddy(%s) called", name);
	int32 reqGroupCount = groups.size();
	Group *master = NULL;
	group_t::iterator gIt;
	group_t newGroups;
	group_t existingGroups;
	
	// Get the master group
	gIt = fGroups.find(0x0000);
	if (gIt == fGroups.end()) {
		LOG(Protocol(), liHigh, "Could not obtain a reference to the master "
			"group, bailing on add SSI buddy");
		return B_ERROR;
	};
	master = gIt->second;
	
	Flap *startTran = new Flap(SNAC_DATA);
	startTran->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_EDIT_BEGIN, 0x00,
		0x00, 0x00000000));
	startTran->AddInt32(0x00010000); // Import, avoids authorisation requests
	Send(startTran);
	
	bool createBuddy = false;
	Buddy *buddy = GetBuddy(name);
	if (buddy == NULL) {
		createBuddy = true;
		buddy = new Buddy(name, GetNewItemId());

		fBuddy[name] = buddy;
	};
	
	for (int32 i = 0; i < reqGroupCount; i++) {
		Group *group = NULL;
		
		for (gIt = fGroups.begin(); gIt != fGroups.end(); gIt++) {
			Group *curGroup = gIt->second;
			if (groups[i] == curGroup->Name()) {
				group = curGroup;			
				break;
			};
		};

		if (group == NULL) {
			group = new Group(GetNewItemId(), groups[i].String());
			group->AddItem(buddy->ItemID());
			
			newGroups[group->Id()] = group;
			fGroups[group->Id()] = group;
			master->AddItem(group->Id());
			
		} else {
			if (buddy->IsInGroup(group->Id()) == false) {
				group->AddItem(buddy->ItemID());
				existingGroups[group->Id()] = group;
			};
		};
	};
	
	// This Buddy isn't in a group already, and doesn't have any groups, throw
	// them in the first group
	if ((reqGroupCount == 0) && (buddy->CountGroups() == 0)) {
		gIt = fGroups.begin();
		existingGroups[gIt->first] = gIt->second;
		buddy->AddGroup(gIt->first);
	};
		
	if (newGroups.empty() == false) {
		Flap *createGroups = new Flap(SNAC_DATA);
		createGroups->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, ADD_SSI_ITEM));
		
		for (gIt = newGroups.begin(); gIt != newGroups.end(); gIt++) {
			Group *group = gIt->second;

			// Create the group entry on the server side
			createGroups->AddInt16(strlen(group->Name()));
			createGroups->AddRawData((uchar *)group->Name(), strlen(group->Name()));
			createGroups->AddInt16(group->Id());	// Group ID
			createGroups->AddInt16(0x0000);			// Item ID
			createGroups->AddInt16(GROUP_RECORD);	// Type

			// Add all the buddy items in this group
			BufferWriter contentWriter;
			for (int32 i = 0; i < group->ItemsInGroup(); i++) {
				int16 value = group->ItemAt(i);
				LOG(Protocol(), liHigh, "%'s contains: 0x%04x\n",
					group->Name(), value);
			
				contentWriter.WriteInt16(value);
			};
				
			TLV *items = new TLV(0x00c8, (char *)contentWriter.Buffer(), contentWriter.Length());
			createGroups->AddInt16(items->FlattenedSize());
			createGroups->AddTLV(items);
			
			// Add the buddy record for this group
			createGroups->AddInt16(strlen(buddy->Name()));
			createGroups->AddRawData((uchar *)buddy->Name(), strlen(buddy->Name()));
			createGroups->AddInt16(group->Id());		// Group ID
			createGroups->AddInt16(buddy->ItemID());	// Buddy ID
			createGroups->AddInt16(BUDDY_RECORD);
			createGroups->AddInt16(0x0000);				// No additional info
		};
	
		// For all the groups that already exist, add the buddy to them
		for (gIt = existingGroups.begin(); gIt != existingGroups.end(); gIt++) {
			Group *group = gIt->second;
			
			createGroups->AddInt16(strlen(buddy->Name()));
			createGroups->AddRawData((uchar *)buddy->Name(), strlen(buddy->Name()));
			createGroups->AddInt16(group->Id());		// Group ID
			createGroups->AddInt16(buddy->ItemID());	// Buddy ID
			createGroups->AddInt16(BUDDY_RECORD);
			createGroups->AddInt16(0x0000);				// No additional info
		};
	
		Send(createGroups);
	};

	if (existingGroups.empty() == false) {
		Flap *modifyGroups = new Flap(SNAC_DATA);
		modifyGroups->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_UPDATE_ITEM));
		
		for (gIt = existingGroups.begin(); gIt != existingGroups.end(); gIt++) {
			Group *group = gIt->second;
			
			modifyGroups->AddInt16(strlen(group->Name()));
			modifyGroups->AddRawData((uchar *)group->Name(), strlen(group->Name()));
			modifyGroups->AddInt16(group->Id());	// Group ID
			modifyGroups->AddInt16(0x0000);			// Item ID
			modifyGroups->AddInt16(GROUP_RECORD);	// Type

			BufferWriter contentWriter;
			for (int32 i = 0; i < group->ItemsInGroup(); i++) {
				int16 value = group->ItemAt(i);
				LOG(Protocol(), liHigh, "%'s contains: 0x%04x\n",
					group->Name(), value);
			
				contentWriter.WriteInt16(value);
			};
			
			// Group contents
			TLV *items = new TLV(0x00c8, (char *)contentWriter.Buffer(), contentWriter.Length());
			modifyGroups->AddInt16(items->FlattenedSize());
			modifyGroups->AddTLV(items);
		};
		
		// Update the Master group's contents
		modifyGroups->AddInt16(0x0000);	// Name length
		modifyGroups->AddInt16(0x0000);	// Group Id
		modifyGroups->AddInt16(0x0000);	// Item Id
		modifyGroups->AddInt16(GROUP_RECORD);
			
		BufferWriter masterWriter;
		for (int32 i = 0; i < master->ItemsInGroup(); i++) {
			int16 value = master->ItemAt(i);
			LOG(Protocol(), liHigh, "Master's %i group: 0x%04x\n",
				i, value);
		
			masterWriter.WriteInt16(value);
		};
		
		TLV *items = new TLV(0x00c8, (char *)masterWriter.Buffer(), masterWriter.Length());
		modifyGroups->AddInt16(items->FlattenedSize());
		modifyGroups->AddTLV(items);		
		
		Send(modifyGroups);
	};
	
	Flap *endTran = new Flap(SNAC_DATA);
	endTran->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_EDIT_END, 0x00,
		0x00, 0x00000000));
	Send(endTran);

	return B_OK;	
};

status_t OSCARManager::AddBuddy(const char *buddy) {
	status_t ret = B_ERROR;
	if (buddy != NULL) {
		LOG(Protocol(), liLow, "OSCARManager::AddBuddy: Adding \"%s\" to list", buddy);
		
		Flap *addBuddy = new Flap(SNAC_DATA);
		addBuddy->AddSNAC(new SNAC(BUDDY_LIST_MANAGEMENT, ADD_BUDDY_TO_LIST));
		
		uint8 buddyLen = strlen(buddy);
		addBuddy->AddInt8(buddyLen);
		addBuddy->AddRawData((uchar *)buddy, buddyLen);
		
		ret = B_OK;
	};

	return ret;
};

status_t OSCARManager::AddBuddies(list <char *>buddies) {
	Flap *addBuds = new Flap(SNAC_DATA);
	addBuds->AddSNAC(new SNAC(BUDDY_LIST_MANAGEMENT, ADD_BUDDY_TO_LIST, 0x00,
		0x00, 0x00000000));
	
	list <char *>::iterator i;
	uint8 buddyLen = 0;
	
	for (i = buddies.begin(); i != buddies.end(); i++) {
		char *buddy = (*i);
		buddyLen = strlen(buddy);

//		fBuddy[buddy] = NULL;		
		
		addBuds->AddRawData((uchar *)&buddyLen, sizeof(buddyLen));
		addBuds->AddRawData((uchar *)buddy, buddyLen);
		
		free(buddy);
	};
	
	Send(addBuds);
	
	return B_OK;	
};

status_t OSCARManager::RemoveBuddy(const char *buddy) {
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
		};
#if 0
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
#endif
	};
	
	return ret;
};

status_t OSCARManager::RemoveBuddies(list <char *>buddy) {
	(void)buddy;
	return B_OK;
};

Buddy *OSCARManager::GetBuddy(const char *screenname) {
	Buddy *buddy = NULL;
	buddymap::iterator bIt = fBuddy.find(screenname);
	if (bIt != fBuddy.end()) buddy = bIt->second;
	
	return buddy;
};

int32 OSCARManager::Buddies(void) const {
	return fBuddy.size();
};

uchar OSCARManager::IsConnected(void) const {
	return fConnectionState;
};

status_t OSCARManager::LogOff(void) {
	status_t ret = B_ERROR;
	if (fConnectionState != OSCAR_OFFLINE) {
		fConnectionState = OSCAR_OFFLINE;
		
		LOG(Protocol(), liLow, "%i connection(s) to kill", fConnections.size());
		
		ClearConnections();
		ClearWaitingSupport();

		fHandler->StatusChanged(fOurNick, OSCAR_OFFLINE);
		ret = B_OK;
	};

	return ret;
};

status_t OSCARManager::TypingNotification(const char *buddy, uint16 typing) {
	LOG(Protocol(), liLow, "Sending typing notification (0x%04x) to \"%s\"",
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

status_t OSCARManager::SetAway(const char *message) {
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

status_t OSCARManager::SetProfile(const char *profile) {
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

status_t OSCARManager::SetIcon(const char *icon, int16 size) {
	LOG(Protocol(), liHigh, "OSCARManager::SetIcon(%p, %i) called", icon, size);

	if (fIcon) free(fIcon);
	fIcon = (char *)calloc(size, sizeof(char));
	memcpy(fIcon, icon, size);
	fIconSize = size;
	
	LOG(Protocol(), liHigh, "fIcon: %p (%i)", fIcon, size);

//	if ((icon == NULL) || (size < 0)) return B_ERROR;
	
	if (fConnectionState == OSCAR_OFFLINE) return B_ERROR;
	
	LOG(Protocol(), liHigh, "OSCARManager::SetIcon() - Passed tests, sending");
	
	Flap *start = new Flap(SNAC_DATA);
	start->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_EDIT_BEGIN));
	Send(start);
	
	Flap *add = new Flap(SNAC_DATA);
	add->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, ADD_SSI_ITEM));
	add->AddInt16(strlen("1"));			// Length of name
//	add->AddRawData((uchar *)"1", strlen("1"));
	add->AddInt8('1');
	add->AddInt16(0x0000);				// Group ID
add->AddInt16(0x0001);
	
//	fIconId = GetNewItemId();
//	LOG(Protocol(), liHigh, "%i IDs, new ID: 0x%04x", fItemIds.size(), id);
//	fItemIds[id] = true;
//	
//	add->AddInt16(fIconId);	// Icon ID
	add->AddInt16(BUDDY_ICON_INFO);
	
	TLV *iconData = new TLV(0x00d5);
	BufferWriter writer;
	writer.WriteInt8(0x01);			// Icon Flag (Upload)
	writer.WriteInt8(16);			// Length of the MD5 hash
	
	char md5Hash[16];
	MD5(icon, size, md5Hash);
	
	writer.WriteData((uchar *)md5Hash, sizeof(md5Hash));
	
	iconData->Value((const char *)writer.Buffer(), writer.Length());

//	TLV *name = new TLV(0x0131, "", strlen(""));
	
	add->AddInt16(iconData->FlattenedSize()); // + name->FlattenedSize());
	add->AddTLV(iconData);
//	add->AddTLV(name);

	Send(add);
	
	Flap *end = new Flap(SNAC_DATA);
	end->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, SSI_EDIT_END));
	Send(end);
	
	return B_OK;

//	if ((icon == NULL) || (size < 0)) return B_ERROR;
//
//	if (fIcon) free(fIcon);
//	fIcon = (char *)calloc(size, sizeof(char));
//	fIconSize = size;
//	memcpy(fIcon, icon, size);
//
//	if (fConnectionState == OSCAR_OFFLINE) return B_ERROR;
//	
//	Flap *add = new Flap(SNAC_DATA);
//	add->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, ADD_SSI_ITEM));
//	add->AddInt16(0x0001);		// Size of name
//	add->AddInt8('3');			// Name
//	add->AddInt16(0x0000);		// Group ID
//	add->AddInt16(0x1813);		// Item ID
//	add->AddInt16(BUDDY_ICON_INFO);
//	add->AddInt16(0x0016);		// Length of additional data
//	
//	char buffer[18];
//	buffer[0] = 0x01; // Icon flags
//	buffer[2] = 0x10; // MD5 Length
//	MD5(icon, size, buffer + 2);
//	add->AddTLV(new TLV(0x00d5, buffer, 18));
//	add->AddTLV(new TLV(0x0131, "", 0));
//	Send(add);
//	
//	return B_OK;
};

uint16 OSCARManager::GetNewItemId(void) {
	uint16 id = 0;
	identity_t::iterator iIt;
	for (iIt = fItemIds.begin(); iIt != fItemIds.end(); iIt++) {
		id = max_c(iIt->first, id);
	};
	
	return id + 1;
};
