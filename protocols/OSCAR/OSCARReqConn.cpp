#include "OSCARReqConn.h"

#include "FLAP.h"
#include "SNAC.h"
#include "TLV.h"
#include "BufferReader.h"

//#pragma mark Constructor

OSCARReqConn::OSCARReqConn(const char *server, uint16 port,	OSCARManager *man)
	: OSCARConnection(server, port, man, "OSCAR ReqConn", connReq) {
	
	fManager = man;
	fManMsgr = BMessenger(fManager);
};

OSCARReqConn::~OSCARReqConn(void) {
};

//#pragma mark Public

//#pragma mark Private
						
status_t OSCARReqConn::HandleServiceControl(SNAC *snac, BufferReader *reader) {
	status_t ret = B_OK;
	
	uint16 subtype = snac->SubType();
	int32 request = snac->RequestID();
	
	reader->OffsetTo(snac->DataOffset());
	
	switch (subtype) {
		case ERROR: {
			LOG(ConnName(), liHigh, "OSCARReqConn (%s:%i) got an error", Server(),
				Port());
			ret = kUnhandled;
		} break;
		case SERVER_SUPPORTED_SNACS: {
			LOG(ConnName(), liLow, "Got server supported SNACs");

			BMessage newCaps(AMAN_NEW_CAPABILITIES);

			while (reader->HasMoreData()) {
				uint16 s = reader->ReadInt16();
				LOG(ConnName(), liLow, "Server supports 0x%04x", s);
				Support(s);
				newCaps.AddInt16("family", s);
			};
			
			fManMsgr.SendMessage(&newCaps);
			
			if (State() == OSCAR_CONNECTING) {
				Flap *f = new Flap(SNAC_DATA);
				f->AddSNAC(new SNAC(SERVICE_CONTROL, FAMILY_VERSIONS));
				
				f->AddInt16(0x0001); // Family 1
				f->AddInt16(0x0003); // Version 3

				f->AddInt16(0x0013);
				f->AddInt16(0x0002);

				f->AddInt16(0x0002);
				f->AddInt16(0x0001);

				f->AddInt16(0x0003);
				f->AddInt16(0x0001);

				f->AddInt16(0x0015);
				f->AddInt16(0x0001);
				
				f->AddInt16(0x0004);
				f->AddInt16(0x0001);

				f->AddInt16(0x0006);
				f->AddInt16(0x0001);
				
				f->AddInt16(0x0009);
				f->AddInt16(0x0001);

				f->AddInt16(0x000a);
				f->AddInt16(0x0001);
				
				f->AddInt16(0x000b);
				f->AddInt16(0x0001);
				Send(f);
			};
		} break;
		
		case SERVER_FAMILY_VERSIONS: {
			Flap *f = new Flap(SNAC_DATA);
			f->AddSNAC(new SNAC(SERVICE_CONTROL, REQUEST_RATE_LIMITS));
			
			Send(f);
		} break;
		
		case RATE_LIMIT_RESPONSE: {
//			We should parse this...

			Flap *f = new Flap(SNAC_DATA);
			f->AddSNAC(new SNAC(SERVICE_CONTROL, RATE_LIMIT_ACK));
			f->AddInt16(0x0001);
			f->AddInt16(0x0002);
			f->AddInt16(0x0003);
			f->AddInt16(0x0004);						
			
			Send(f);
			
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
			
			SetState(OSCAR_ONLINE);
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

status_t OSCARReqConn::HandleBuddyIcon(SNAC *snac, BufferReader *reader) {
	status_t ret = kUnhandled;

	return ret;
};
