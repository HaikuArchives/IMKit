#ifndef FLAP_H
#define FLAP_H

#include <support/SupportDefs.h>
#include <list>

#include "OSCARConstants.h"
#include "TLV.h"
#include "SNAC.h"

typedef std::pair<uchar *, uint32> BufferLengthPair;
typedef std::pair<void *, uint8> TypeDataPair;

extern const uint8 kFLAPHeader;

class BufferReader;

class Flap {
	public:

	enum {
		DATA_TYPE_RAW = 1,
		DATA_TYPE_TLV = 2,
		DATA_TYPE_SNAC = 3
	};

							Flap(uint8 channel);
							Flap(uchar *data, uint16 length);
							Flap(void);
							~Flap(void);
		void				Channel(uint8);
		uint8				Channel(void) const;
		status_t			AddRawData(unsigned char *data, uint16 length);
		status_t			AddInt8(int8 value);
		status_t			AddInt16(int16 value);
		status_t			AddInt32(int32 value);
		status_t			AddInt64(int64 value);
		status_t			AddTLV(TLV *data);
		status_t			AddTLV(uint16 type, const char *value, uint16 length);
		status_t			AddSNAC(SNAC *snac);
		const char			*Flatten(uint16 seqNum);
		uint32				FlattenedSize(void);
		void				Clear(void);
		SNAC				*SNACAt(uint8 index = 0);

		BufferReader		*Reader(void);

	private:
		bool				fDirty;
		uint32				fFlattenedSize;
		char				*fFlat;
		uint8				fChannelID;
		uint16				fSequenceID;
		uint16				fLength;		
		std::list<TypeDataPair>	fData;
};

#endif
