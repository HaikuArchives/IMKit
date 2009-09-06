#include "SNAC.h"

#include <common/support/BufferReader.h>

SNAC::SNAC(void) {
	fFamily = 0;
	fSubType = 0;
	fFlags = 0;
	fRequestID = 0;
	fDirty = true;
	
	fDataOffset = 0;
};

SNAC::SNAC(uint16 family, uint16 subtype, uint8 flag1, uint8 flag2,
	uint32 request) {
	
	fFamily = family;
	fSubType = subtype;
	fFlags = (flag1 << 8) | flag2;
	fRequestID = request;
	
	fDataOffset = 0;
	
	fDirty = true;
};

SNAC::SNAC(BufferReader *reader) {
	fFamily = reader->ReadInt16();
	fSubType = reader->ReadInt16();
	fFlags = reader->ReadInt16();
	fRequestID = reader->ReadInt32();
	
	fDirty = true;

	fDataOffset = 0;

	if (fFlags & 0x8000) {
		int32 readerOffset = reader->Offset();
		int16 offset = reader->ReadInt16();
		fDataOffset = offset + sizeof(offset);
		
		reader->OffsetTo(readerOffset);
	};
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
	return fFlags;
};

void SNAC::Flags(uint8 flag1, uint8 flag2) {
	fDirty = true;
	fFlags = (flag1 << 8) | flag2;
};

void SNAC::Flags(uint16 flags) {
	fDirty = true;
	fFlags = flags;
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

		fFlattened[4] = (fFlags & 0xff00) << 8;
		fFlattened[5] = fFlags & 0x00ff;
		
		fFlattened[6] = (fRequestID & 0xff000000) << 24;
		fFlattened[7] = (fRequestID & 0x00ff0000) << 16;
		fFlattened[8] = (fRequestID & 0x0000ff00) << 8;
		fFlattened[9] = (fRequestID & 0x000000ff);
	};
	
	return fFlattened;
};

int16 SNAC::FlattenedSize() const {
	return 10;
};

int16 SNAC::DataOffset(void) {
	return FlattenedSize() + fDataOffset;
};
