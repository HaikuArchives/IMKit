#ifndef BUFFERWRITER_H
#define BUFFERWRITER_H

#include <support/ByteOrder.h>

class BufferWriter {
	public:
							BufferWriter(swap_action swap = B_SWAP_BENDIAN_TO_HOST,
								int32 chunkSize = 50);
							~BufferWriter(void);

		// Accessor Methods							
		uchar				*Buffer(void);
		int32				Length(void);
		int32				Offset(void);
		void				OffsetTo(int32 to);
		void				OffsetBy(int32 by);
		
		// Writer Methods
		void				WriteInt8(int8 value);
		void				WriteInt16(int16 value);
		void				WriteInt32(int32 value);
		void				WriteInt64(int64 value);
		void				WriteData(const uchar *data, int16 length);
		void				WriteString(const char *data, int16 length);
							
	private:
		void				Allocate(int16 length);
		
		uchar				*fBuffer;
		int32				fLength;
		int32				fAllocated;
		int32				fOffset;
		swap_action			fSwap;
		int32				fChunkSize;
};

#endif
