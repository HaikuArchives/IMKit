#include "FLAP.h"

#include <support/ByteOrder.h>
#include <stdio.h>
#include <libim/Helpers.h>

#include "BufferReader.h"

//#pragma mark Constants

const uint8 kFLAPHeader = 6;

//#pragma mark Constructor

Flap::Flap(uint8 channel) {
	fChannelID = channel;
	fDirty = true;
	fFlattenedSize = 0;
	fFlat = NULL;
};

Flap::Flap(uchar *data, uint16 length)
	: fFlat(NULL),
	fFlattenedSize(0),
	fDirty(true) {
	BufferReader reader(data, length, false);
	fChannelID = reader.ReadInt8();
	fSequenceID = reader.ReadInt16();
	int16 datasize = reader.ReadInt16();

	if (fChannelID == SNAC_DATA) {
		AddSNAC(new SNAC(reader.ReadInt16(), reader.ReadInt16(), reader.ReadInt16(),
			reader.ReadInt32()));
	} else {
		AddRawData(data + reader.Offset(), datasize);
	};	

};


Flap::Flap(void) {
	fDirty = true;
	fFlattenedSize = 0;
	fFlat = NULL;
};

Flap::~Flap() {
	Clear();
	if (fFlat != NULL) free(fFlat);
};

//#pragma mark Public Methods

status_t Flap::AddInt8(int8 value) {
	return AddRawData((uchar *)&value, sizeof(value));
};

status_t Flap::AddInt16(int16 value) {
	int16 v = B_HOST_TO_BENDIAN_INT16(value);
	return AddRawData((uchar *)&v, sizeof(value));
};

status_t Flap::AddInt32(int32 value) {
	int32 v = B_HOST_TO_BENDIAN_INT32(value);
	return AddRawData((uchar *)&v, sizeof(value));
};

status_t Flap::AddInt64(int64 value) {
	int64 v = B_HOST_TO_BENDIAN_INT64(value);
	return AddRawData((uchar *)&v, sizeof(value));
};


status_t Flap::AddRawData(unsigned char *data, uint16 length) {
	fDirty = true;
	uchar *t = (uchar *)calloc(length, sizeof(uchar));
	memcpy(t, data, length);

	BufferLengthPair *blp = new BufferLengthPair(t, length);
	TypeDataPair p((void *)blp, DATA_TYPE_RAW);
	fData.push_back(p);

	return B_OK;
};

status_t Flap::AddTLV(TLV *tlv) {
	fDirty = true;
	TypeDataPair p((void *)tlv, DATA_TYPE_TLV);
	fData.push_back(p);

	return B_OK;
};

status_t Flap::AddTLV(uint16 type, const char *value, uint16 length) {
	return AddTLV(new TLV(type, value, length));
};

status_t Flap::AddSNAC(SNAC *snac) {
	fDirty = true;
	TypeDataPair p((void *)snac, DATA_TYPE_SNAC);
	fData.push_back(p);
	
	return B_OK;
};

uint32 Flap::FlattenedSize(void) {
	uint32 r = 0;
	if (fDirty) {
		list <TypeDataPair>::iterator i;
		fFlattenedSize = 0;
		
		
		for (i = fData.begin(); i != fData.end(); i++) {
			TypeDataPair p = (*i);
			switch (p.second) {
				case DATA_TYPE_RAW: {
					BufferLengthPair *blp = (BufferLengthPair *)p.first;
					fFlattenedSize += blp->second;
				} break;
				
				case DATA_TYPE_TLV: {
					TLV *t = (TLV *)p.first;
					fFlattenedSize += t->FlattenedSize();
				} break;
				
				case DATA_TYPE_SNAC: {
					SNAC *s = (SNAC *)p.first;
					fFlattenedSize += s->FlattenedSize();
				} break;
			};
		};
		
		fFlattenedSize += kFLAPHeader;
		
	};

	r = fFlattenedSize;
	
	return r;
};

const char *Flap::Flatten(uint16 seqNum) {
	if (fDirty) {
		LOG("AIM", liDebug, "Sequence 0x%04x: Need to allocate %i bytes...",
			seqNum, FlattenedSize());
		fFlat = (char *)realloc(fFlat, FlattenedSize());
		if (fFlat == NULL) {
			LOG("AIM", liHigh, "Could not allocate memory for flap");
			return NULL;
		};
		fDirty = false;
			
		list <TypeDataPair>::iterator i;
		uint32 offset = 0;
		
		fFlat[0] = 0x2a;
		fFlat[1] = fChannelID;
		fFlat[2] = (seqNum & 0xff00) >> 8;
		fFlat[3] = (seqNum & 0x00ff);
		fFlat[4] = ((FlattenedSize() - kFLAPHeader) & 0xff00) >> 8;
		fFlat[5] = ((FlattenedSize() - kFLAPHeader) & 0x00ff);
		
		offset += kFLAPHeader;
		
		for (i = fData.begin(); i != fData.end(); i++) {
			TypeDataPair p = (*i);

			switch (p.second) {
				case DATA_TYPE_RAW: {
					BufferLengthPair *blp = (BufferLengthPair *)p.first;
					memcpy((void *)(fFlat + offset), blp->first, blp->second);
					offset += blp->second;

				} break;
				
				case DATA_TYPE_TLV: {
					TLV *t = (TLV *)p.first;
					memcpy((void *)(fFlat + offset), t->Flatten(), t->FlattenedSize());
					offset += t->FlattenedSize();
				} break;
				
				case DATA_TYPE_SNAC: {
					SNAC *s = (SNAC *)p.first;
					memcpy((void *)(fFlat + offset), s->Flatten(), s->FlattenedSize());
					offset += s->FlattenedSize();
				} break;
			};
		};

	};

	return fFlat;
};

uint8 Flap::Channel(void) const {
	return fChannelID;
};

void Flap::Channel(uint8 channel) {
	fDirty = true;
	fChannelID = channel;
};

void Flap::Clear(void) {
	list <TypeDataPair>::iterator i;

	for (i = fData.begin(); i != fData.end(); i++) {

		TypeDataPair p = (*i);
		switch (p.second) {
			case DATA_TYPE_RAW: {
				BufferLengthPair *blp = (BufferLengthPair *)p.first;
				free(blp->first);
				delete blp;
			} break;
			
			case DATA_TYPE_TLV: {
				delete (TLV *)p.first;
			} break;
			
			case DATA_TYPE_SNAC: {
				delete (SNAC *)p.first;
			} break;
		};
//		Should the TypeDataPair itself be deleted? I presume so, it was new'd.
	};

	fData.clear();

	if (fFlat != NULL) free(fFlat);
	fFlat = NULL;
	fDirty = true;
};

SNAC *Flap::SNACAt(uint8 index = 0) {
	uint8 snacCount = 0;
	
	list <TypeDataPair>::iterator i;
	
	for (i = fData.begin(); i != fData.end(); i++) {
		TypeDataPair p = (*i);
		
		if (p.second == DATA_TYPE_SNAC) {
			if (snacCount == index) return (SNAC *)p.first;
			snacCount++;
		};
	};
	
	return NULL;
};

BufferReader *Flap::Reader(void) {
	if (fDirty) Flatten(fSequenceID);
	return new BufferReader((uchar *)fFlat, fFlattenedSize, false);
};
