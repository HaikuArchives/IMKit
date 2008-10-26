#include "SNAC.h"

SNAC::SNAC(void) {
	fFamily = 0;
	fSubType = 0;
	fFlags[0] = 0;
	fFlags[1] = 0;
	fRequestID = 0;
	fDirty = true;
};

SNAC::SNAC(uint16 family, uint16 subtype, uint8 flag1 = 0x00, uint8 flag2 = 0x00,
	uint32 request = 0x00000000) {
	
	fFamily = family;
	fSubType = subtype;
	fFlags[0] = flag1;
	fFlags[1] = flag2;
	fRequestID = request;
	fDirty = true;
};

SNAC::~SNAC(void) {
};

uint16 SNAC::Family(void) const {
	return fFamily;
};

void SNAC::Family(uint16 family) {
	fDirty = true;
	fFamily = family;
};

uint16 SNAC::SubType(void) const {
	return fSubType;
};

void SNAC::SubType(uint16 subtype) {
	fDirty = true;
	fSubType = subtype;
};

uint16 SNAC::Flags(void) const {
	return (fFlags[0] << 8) + fFlags[1];
};

void SNAC::Flags(uint8 flag1, uint8 flag2) {
	fDirty = true;
	fFlags[0] = flag1;
	fFlags[1] = flag2;
};

void SNAC::Flags(uint16 flags) {
	fDirty = true;
	fFlags[0] = (flags & 0xff00) << 8;
	fFlags[1] = (flags & 0x00ff);
};

uint32 SNAC::RequestID(void) const {
	return fRequestID;
};

void SNAC::RequestID(uint32 reqid) {
	fDirty = true;
	fRequestID = reqid;
};
			
char *SNAC::Flatten(void) {
	if (fDirty) {
		fFlattened[0] = (fFamily & 0xff00) << 8;
		fFlattened[1] = fFamily & 0x00ff;
		
		fFlattened[2] = (fSubType & 0xff00) << 8;
		fFlattened[3] = fSubType & 0x00ff;

		fFlattened[4] = fFlags[0];
		fFlattened[5] = fFlags[1];
		
		fFlattened[6] = (fRequestID & 0xff000000) << 24;
		fFlattened[7] = (fRequestID & 0x00ff0000) << 16;
		fFlattened[8] = (fRequestID & 0x0000ff00) << 8;
		fFlattened[9] = (fRequestID & 0x000000ff);
	};
	
	return fFlattened;
};

