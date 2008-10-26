#include "AIMBOSConnection.h"

#include <UTF8.h>
#include <sys/select.h>

void remove_html(char *msg);
void PrintHex(const unsigned char* buf, size_t size, bool override = false);

AIMBOSConnection::AIMBOSConnection(const char *server, uint16 port, AIMManager *man,
	const char *name = "AIMBOS Connection")
	: OSCARConnection(server, port, man, name, connBOS) {

	fManager = man;
	fManMsgr = BMessenger(fManager);
};

AIMBOSConnection::~AIMBOSConnection(void) {
};

//#pragma mark -
// These are the virtual handlers

status_t AIMBOSConnection::HandleServiceControl(BMessage *msg) {
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

	offset = 9;
	
	if (flags & 0x8000) {
		uint16 skip = (data[++offset] << 8) + data[++offset];
		offset += skip;
	};

	switch (subtype) {
		case ERROR: {
			ret = kUnhandled;
		} break;
		case VERIFICATION_REQUEST: {
		} break;
			
		case SERVER_SUPPORTED_SNACS: {
			LOG(ConnName(), liLow, "Got server supported SNACs");

			while (offset < bytes) {
				uint16 s = (data[++offset] << 8) + data[++offset];
				LOG(ConnName(), liLow, "Server supports 0x%04x", s);
				Support(s);
			};

			fManMsgr.SendMessage(AMAN_NEW_CAPABILITIES);
			
			if (State() == OSCAR_CONNECTING) {
				Flap *f = new Flap(SNAC_DATA);
				f->AddSNAC(new SNAC(SERVICE_CONTROL, REQUEST_RATE_LIMITS));
				Send(f);
				
				fManager->Progress("AIM Login", "AIM: Got server capabilities", 0.5);
			};
		} break;
		
		case SERVICE_REDIRECT: {
			LOG(ConnName(), liLow, "Got service redirect SNAC");
			uint16 tlvType = 0;
			uint16 tlvLen = 0;
			char *tlvValue = NULL;
						
			BMessage service(AMAN_NEW_CONNECTION);
		
			while (offset < bytes) {
				tlvType = (data[++offset] << 8) + data[++offset];
				tlvLen = (data[++offset] << 8) + data[++offset];
				tlvValue = (char *)calloc(tlvLen + 1, sizeof(char));
				memcpy(tlvValue, (void *)(data + offset + 1), tlvLen);
				tlvValue[tlvLen] = '\0';
				
				offset += tlvLen;

				switch(tlvType) {
					case 0x000d: {	// Service Family
						uint16 family = (tlvValue[0] << 8) + tlvValue[1];						
						service.AddInt16("family", family);
					} break;
					
					case 0x0005: {	// Server Details
						LOG(ConnName(), liLow, "Server details: %s\n", tlvValue);

						if (strchr(tlvValue, ':')) {
							pair<char *, uint16> sd = ExtractServerDetails(tlvValue);
							service.AddString("host", sd.first);
							service.AddInt16("port", sd.second);
							free(sd.first);
						} else {
							service.AddString("host", tlvValue);
							service.AddInt16("port", 5190);
						};
						
					} break;
					
					case 0x0006: {	// Cookie, nyom nyom nyom!
						LOG(ConnName(), liLow, "Cookie");
						service.AddData("cookie", B_RAW_TYPE, tlvValue, tlvLen);
					} break;
					
					default: {
					};
				};
				
				free(tlvValue);
			};					
			fManMsgr.SendMessage(&service);
			
		} break;
		
		case RATE_LIMIT_RESPONSE: {
			Flap *f = new Flap(SNAC_DATA);
			f->AddSNAC(new SNAC(SERVICE_CONTROL, RATE_LIMIT_ACK));
			f->AddInt16(0x0001);
			f->AddInt16(0x0002);
			f->AddInt16(0x0003);
			f->AddInt16(0x0004);						
			Send(f);
			
			if (State() != OSCAR_CONNECTING) return B_OK;
			
			fManager->Progress("AIM Login", "AIM: Got rate limits", 0.6);
			
//			Server won't respond to the Rate ACK above
//			Carry on with login (Send Privacy bits)
			Flap *SPF = new Flap(SNAC_DATA);
			SPF->AddSNAC(new SNAC(SERVICE_CONTROL, SET_PRIVACY_FLAGS));
			SPF->AddInt32(0x00000003); // View everything
			Send(SPF);
			
//			And again... all these messages will be
//			replied to as the server sees fit
			Flap *OIR = new Flap(SNAC_DATA);
			OIR->AddSNAC(new SNAC(SERVICE_CONTROL, UPDATE_STATUS));
			Send(OIR);

			Flap *buddy = new Flap(SNAC_DATA);
			buddy->AddSNAC(new SNAC(BUDDY_LIST_MANAGEMENT, ADD_BUDDY_TO_LIST));
			buddy->AddInt8(0x00);

			Flap *cap = new Flap(SNAC_DATA);
			cap->AddSNAC(new SNAC(LOCATION, SET_USER_INFORMATION));

			const char *profile = fManager->Profile();
			if (profile != NULL) {
				cap->AddTLV(new TLV(0x0001, kEncoding, strlen(kEncoding)));
				cap->AddTLV(new TLV(0x0002, profile, strlen(profile)));
			};
			cap->AddTLV(0x0005, kBuddyIconCap, kBuddyIconCapLen);
			Send(cap);
			
			
			Flap *icbm = new Flap(SNAC_DATA);
			icbm->AddSNAC(new SNAC(ICBM, SET_ICBM_PARAMS));
			icbm->AddInt16(0x0001);
			icbm->AddInt32(0x00000009);
			icbm->AddInt16(0x1f40);// Max SNAC
			icbm->AddInt16(0x03e7);// Max Warn - send
			icbm->AddInt16(0x03e7);// Max Warn - Recv
			icbm->AddInt16(0x0000);// Min Message interval (sec);
			icbm->AddInt16(0x0064);//??

			Send(icbm);
			
			Flap *cready = new Flap(SNAC_DATA);
			cready->AddSNAC(new SNAC(SERVICE_CONTROL, CLIENT_READY));
			cready->AddInt16(0x0001);	// Family 1
			cready->AddInt16(0x0003);	// Version
			cready->AddInt16(0x0110);	// Tool ID
			cready->AddInt16(0x0739);	// Tool Version

			cready->AddInt16(0x0002);
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);
			
			cready->AddInt16(0x0003);
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);

			cready->AddInt16(0x0004);
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);

			cready->AddInt16(0x0006);
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);

			cready->AddInt16(0x0008);
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0104);
			cready->AddInt16(0x0001);

			cready->AddInt16(0x0009);
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);

			cready->AddInt16(0x0010);	// Remove?
			cready->AddInt16(0x0001);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);

			cready->AddInt16(0x0013);
			cready->AddInt16(0x0003);
			cready->AddInt16(0x0110);
			cready->AddInt16(0x0739);

			Send(cready);
			
			Flap *ssiparam = new Flap(SNAC_DATA);
			ssiparam->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, REQUEST_PARAMETERS));
			Send(ssiparam);
			
			Flap *ssi = new Flap(SNAC_DATA);
			ssi->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, REQUEST_LIST));
			Send(ssi);
			
			Flap *useSSI = new Flap(SNAC_DATA);
			useSSI->AddSNAC(new SNAC(SERVER_SIDE_INFORMATION, ACTIVATE_SSI_LIST));
			Send(useSSI);
			
			fManager->Progress("AIM Login", "AIM: Requested server"
				" side buddy list", 1.0);
			
			BMessage status(AMAN_STATUS_CHANGED);
			status.AddInt8("status", OSCAR_ONLINE);

			fManMsgr.SendMessage(&status);
			SetState(OSCAR_ONLINE);
		
		} break;
		case SERVER_FAMILY_VERSIONS: {
			LOG(ConnName(), liLow, "Supported SNAC families for "
				"this server");
			while (offset < bytes) {
				LOG(ConnName(), liLow, "\tSupported family: 0x%x "
					" 0x%x, Version: 0x%x 0x%x", data[++offset],
					data[++offset], data[++offset], data[++offset]);
			};
		} break;

		case EXTENDED_STATUS: {
			ret = kUnhandled;
		} break;

		default: {
			ret = kUnhandled;
		};
	};
	
	return ret;
};

status_t AIMBOSConnection::HandleAuthorisation(BMessage *msg) {
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

	offset = 9;
	
	if (flags & 0x8000) {
		uint16 skip = (data[++offset] << 8) + data[++offset];
		offset += skip;
	};
	
	msg->PrintToStream();

	switch (subtype) {
		case MD5_KEY_REPLY: {
			PrintHex(data, bytes, true);
			ret = B_OK;
		} break;
		
	};
	
	return ret;
};
