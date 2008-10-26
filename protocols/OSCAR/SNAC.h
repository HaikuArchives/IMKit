#ifndef SNAC_H
#define SNAC_H

#include <support/SupportDefs.h>

class BufferReader;

class SNAC {
	public:
						SNAC(void);
						SNAC(uint16 family, uint16 subtype, uint8 flag1 = 0x00,
							uint8 flag2 = 0x00, uint32 request = 0x00000000);
						SNAC(BufferReader *reader);
						~SNAC(void);

		uint16			Family(void) const;
		void			Family(uint16 family);
		uint16			SubType(void) const;
		void			SubType(uint16 subtype);
		uint16			Flags(void) const;
		void			Flags(uint8 flag1, uint8 flag2);
		void			Flags(uint16 flags);
		uint32			RequestID(void) const;
		void			RequestID(uint32);
		char			*Flatten(void);
		int16			FlattenedSize(void) const;
			
		int16			DataOffset(void);
		BufferReader	*Reader(void);

	private:
		uint16			fFamily;
		uint16			fSubType;
		uint16			fFlags;
		uint32			fRequestID;
		bool			fDirty;
		char 			fFlattened[10];
		int16			fDataOffset;
};

#endif
