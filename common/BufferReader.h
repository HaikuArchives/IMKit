#ifndef BUFFERREADER_H
#define BUFFERREADER_H

#include <support/ByteOrder.h>

class BufferReader {
	public:
							BufferReader(const uchar *data, int32 length,
								swap_action swap = B_SWAP_BENDIAN_TO_HOST);
							BufferReader(uchar *data, int32 length, bool own = true,
								swap_action swap = B_SWAP_BENDIAN_TO_HOST);
							~BufferReader(void);

		// Accessor Methods							
		uchar				*Buffer(void) const;
		int32				Length(void) const;
		int32				Offset(void);
		void				OffsetTo(int32 to);
		void				OffsetBy(int32 by);
		bool				HasMoreData(void) const;
		
		// Reader Methods
		int8				ReadInt8(bool autoincrement = true);
		int16				ReadInt16(bool autoincrement = true);
		int32				ReadInt32(bool autoincrement = true);
		int64				ReadInt64(bool autoincrement = true);
		uchar				*ReadData(int16 length, bool autoincrement = true);
		char				*ReadString(int16 length, bool autoincrement = true);
		
		void				Debug(bool fromOffset = false);
		
		// Operators
		bool				operator == (const BufferReader &rhs);
		bool				operator == (const BufferReader *rhs);
		
	private:
		uchar				*fBuffer;
		int32				fLength;
		bool				fOwn;
		int32				fOffset;
		swap_action			fSwap;
};

#endif
