#include "OSCARBOSConnection.h"

#include "FLAP.h"
#include "SNAC.h"
#include "TLV.h"
#include "common/BufferReader.h"
#include "OSCARConnection.h"

#include <stdlib.h>

#include <UTF8.h>

//#pragma mark Constants

const char kBuddyIconCapability[] = {
	0x09, 0x46, 0x13, 0x46,
	0x4c, 0x7f, 0x11, 0xd1,
	0x82, 0x22, 0x44, 0x45,
	0x53, 0x54, 0x00, 0x00
};
const char kCrossProtocolCapability[] = {
	0x09, 0x46, 0x13, 0x4d,
	0x4c, 0x7f, 0x11, 0xd1,
	0x82, 0x22, 0x44, 0x45,
	0x53, 0x54, 0x00, 0x00
};

const uint16 kClientCapabilityCount = 2;

const char *kClientCapabilities[] = {
	kBuddyIconCapability,
	kCrossProtocolCapability
};			


//#pragma mark Functions

void remove_html(char *msg);

//#pragma mark Constructor

OSCARBOSConnection::OSCARBOSConnection(const char *server, uint16 port, OSCARManager *man,
	const char *name)
	: OSCARConnection(server, port, man, name, connBOS) {

	fManager = man;
	fManMsgr = BMessenger(fManager);
};

OSCARBOSConnection::~OSCARBOSConnection(void) {
};

//#pragma mark OSCARConnection Hooks

status_t OSCARBOSConnection::HandleServiceControl(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;
	
	uint16 subtype = snac->SubType();
	//uint32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());

	switch (subtype) {
		case ERROR: {
			ret = kUnhandled;
		} break;
		case VERIFICATION_REQUEST: {
		} break;
			
		case SERVER_SUPPORTED_SNACS: {
			LOG(ConnName(), liLow, "Got server supported SNACs");

			while (reader->Offset() < reader->Length()) {
				uint16 family = reader->ReadInt16();
				LOG(ConnName(), liLow, "Server supports 0x%04x", family);
				Support(family);
			};
			
			fManMsgr.SendMessage(AMAN_NEW_CAPABILITIES);
			
			if (State() == OSCAR_CONNECTING) {
				Flap *f = new Flap(SNAC_DATA);
				f->AddSNAC(new SNAC(SERVICE_CONTROL, REQUEST_RATE_LIMITS));
				Send(f);
				
				fManager->Progress("OSCAR Login", "OSCAR: Got server capabilities", 0.5);
			};
		} break;
		
		case SERVICE_REDIRECT: {
			LOG(ConnName(), liLow, "Got service redirect SNAC");
					
			BMessage service(AMAN_NEW_CONNECTION);
		
			while (reader->Offset() < reader->Length()) {
				TLV tlv(reader);
				BufferReader *tlvReader = tlv.Reader();
				
				switch (tlv.Type()) {
					case 0x000d: {	// Service family
						uint16 family = tlvReader->ReadInt16();
						service.AddInt16("family", family);
					} break;
					
					case 0x0005: {	// Server details
						char *rawdetails = tlvReader->ReadString(tlv.Length());
					
						LOG(ConnName(), liLow, "Redirect server details: %s\n", rawdetails);
						if (strchr(rawdetails, ':')) {
							ServerAddress details = ExtractServerDetails(rawdetails);
							service.AddString("host", details.first);
							service.AddInt16("port", details.second);
							free(details.first);
						} else {
							service.AddString("host", rawdetails);
							service.AddInt16("port", 5190);
						};
						
						free(rawdetails);
					} break;
					
					case 0x0006: {	// Server cookie
						LOG(ConnName(), liDebug, "Got server cookie. Nyom!");
						uchar *cookie = tlvReader->ReadData(tlv.Length());
						service.AddData("cookie", B_RAW_TYPE, cookie, tlv.Length());
						
						free(cookie);
					} break;
					
					default: {
						LOG(ConnName(), liDebug, "Got unhandled TLV (0x%04x) in Service Redirect",
							tlv.Type());
					} break;
				};
				
				delete tlvReader;
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
			
			fManager->Progress("OSCAR Login", "OSCAR: Got rate limits", 0.6);
			
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

//			cap->AddTLV(0x0005, kBuddyIconCap, kCapLen);
			
			char caps[kCapabilityLen * kClientCapabilityCount];
			for (int32 i = 0; i < kClientCapabilityCount; i++) {
				memcpy(caps + (i * kCapabilityLen), kClientCapabilities[i], kCapabilityLen);
			};
			cap->AddTLV(0x0005, caps, kCapabilityLen * kClientCapabilityCount);
			Send(cap);
			
			Flap *icbm = new Flap(SNAC_DATA);
			icbm->AddSNAC(new SNAC(ICBM, SET_ICBM_PARAMS));
			icbm->AddInt16(0x0001);
			icbm->AddInt32(0x00000009);
			icbm->AddInt16(0x1f40);		// Max SNAC
			icbm->AddInt16(0x03e7);		// Max Warn - send
			icbm->AddInt16(0x03e7);		// Max Warn - Recv
			icbm->AddInt16(0x0000);		// Min Message interval (sec);
			icbm->AddInt16(0x0064);		//??

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
			
			fManager->Progress("OSCAR Login", "OSCAR: Requested server side buddy list", 1.0);
			
			BMessage status(AMAN_STATUS_CHANGED);
			status.AddInt8("status", OSCAR_ONLINE);

			fManMsgr.SendMessage(&status);
			SetState(OSCAR_ONLINE);
		
		} break;
		case SERVER_FAMILY_VERSIONS: {
			LOG(ConnName(), liLow, "Supported SNAC families for this server");
			while (reader->Offset() < reader->Length()) {
				uint16 family = reader->ReadInt16();
				uint16 version = reader->ReadInt16();
				
				LOG(ConnName(), liLow, "\tSupported family 0x%04x (V: 0x%04x)",
					family, version);
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

status_t OSCARBOSConnection::HandleAuthorisation(SNAC *snac, BufferReader *reader) {
	status_t ret = kUnhandled;
	
	uint16 subtype = snac->SubType();
	//uint32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());

	switch (subtype) {
		case MD5_KEY_REPLY: {
			reader->Debug();
			ret = B_OK;
		} break;
	};
	
	return ret;
};
