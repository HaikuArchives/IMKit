/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 */

#include <stdlib.h>
#include <string.h>

#include "BufferWriter.h"


//	#pragma mark Constructor


BufferWriter::BufferWriter(swap_action swap, int32 chunkSize)
	: fBuffer(NULL),
	fLength(0),
	fAllocated(0),
	fOffset(0),
	fSwap(swap),
	fChunkSize(chunkSize)
{
	fBuffer = (uchar*)calloc(10000, sizeof(uchar));
	fAllocated = 10000;
}


BufferWriter::~BufferWriter()
{
	free(fBuffer);
};


//	#pragma mark Accessor Methods


uchar*
BufferWriter::Buffer()
{
	return fBuffer;
}


int32
BufferWriter::Length()
{
	return fLength;
}


int32
BufferWriter::Offset()
{
	return fOffset;
}


void
BufferWriter::OffsetTo(int32 to)
{
	fOffset = to;
}


void
BufferWriter::OffsetBy(int32 by)
{
	fOffset += by;
}


//	#pragma mark Reader Methods


void
BufferWriter::WriteInt8(int8 value)
{
	Allocate(sizeof(int8));
	memcpy(fBuffer + fOffset, (void*)&value, sizeof(int8));
	fOffset += sizeof(int8);
}


void
BufferWriter::WriteInt16(int16 value)
{
	Allocate(sizeof(int16));
	swap_data(B_INT16_TYPE, (void*)&value, 1, fSwap);
	memcpy(fBuffer + fOffset, (void*)&value, sizeof(int16));
	fOffset += sizeof(int16);
}


void
BufferWriter::WriteInt32(int32 value)
{
	Allocate(sizeof(int32));
	swap_data(B_INT32_TYPE, (void*)&value, 1, fSwap);
	memcpy(fBuffer + fOffset, (void*)&value, sizeof(int32));
	fOffset += sizeof(int32);
}


void
BufferWriter::WriteInt64(int64 value)
{
	Allocate(sizeof(int64));
	swap_data(B_INT64_TYPE, (void*)&value, 1, fSwap);
	memcpy(fBuffer + fOffset, (void*)&value, sizeof(int64));
	fOffset += sizeof(int64);
}


void
BufferWriter::WriteData(const uchar* data, int16 length)
{
	Allocate(length);
	memcpy(fBuffer + fOffset, data, length);
	fOffset+= length;
}


void
BufferWriter::WriteString(const char* data, int16 length)
{
	Allocate(length);
	memcpy(fBuffer + fOffset, data, length);
	fOffset += length;
}


//	#pragma mark Private Methods


void
BufferWriter::Allocate(int16 length)
{
	if ((fOffset + length) >= fAllocated) {
		int16 alloc = max_c(fChunkSize, length);
		fBuffer = (uchar *)realloc(fBuffer, sizeof(uchar) * alloc);
		fAllocated += alloc;
	}

	fLength = max_c(fOffset + length, fLength);
}
