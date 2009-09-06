/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BufferReader.h"


//	#pragma mark Constructor


BufferReader::BufferReader(const uchar* data, int32 length, swap_action swap)
	: fBuffer((uchar*)data),
	fLength(length),
	fOwn(false),
	fSwap(swap),
	fOffset(0)
{
}


BufferReader::BufferReader(uchar* data, int32 length, bool own,	swap_action swap)
	: fBuffer(data),
	fLength(length),
	fOwn(own),
	fSwap(swap),
	fOffset(0)
{
}


BufferReader::~BufferReader()
{
	if (fOwn)
		free(fBuffer);
}


//	#pragma mark Accessor Methods


uchar*
BufferReader::Buffer() const
{
	return fBuffer;
}


int32
BufferReader::Length() const
{
	return fLength;
}


int32
BufferReader::Offset()
{
	return fOffset;
}


void
BufferReader::OffsetTo(int32 to)
{
	fOffset = to;
}


void
BufferReader::OffsetBy(int32 by)
{
	fOffset += by;
}


bool
BufferReader::HasMoreData() const
{
	return fOffset < fLength;
}


//	#pragma mark Reader Methods

                                        			
int8
BufferReader::ReadInt8(bool autoincrement)
{
	int8 value = *(int8*)(fBuffer + fOffset);
	if (autoincrement)
		fOffset += sizeof(value);

	return value;
}


int16
BufferReader::ReadInt16(bool autoincrement)
{
	int16 value = *(int16*)(fBuffer + fOffset);
	if (autoincrement)
		fOffset += sizeof(value);

	swap_data(B_INT16_TYPE, (void*)&value, 1, fSwap);

	return value;
}


int32
BufferReader::ReadInt32(bool autoincrement)
{
	int32 value = *(int32 *)(fBuffer + fOffset);
	if (autoincrement)
		fOffset += sizeof(value);

	swap_data(B_INT32_TYPE, (void*)&value, 1, fSwap);
	
	return value;
}


int64
BufferReader::ReadInt64(bool autoincrement)
{
	int64 value = *(int64*)(fBuffer + fOffset);
	if (autoincrement)
		fOffset += sizeof(value);

	swap_data(B_INT64_TYPE, (void*)&value, 1, fSwap);

	return value;
}


uchar*
BufferReader::ReadData(int16 length, bool autoincrement)
{
	uchar* buffer = NULL;
	if (length > 0) {
		buffer = (uchar *)calloc(length, sizeof(uchar));
		memcpy(buffer, fBuffer + fOffset, length);

		if (autoincrement)
			fOffset += length;
	}

	return buffer;
}


char*
BufferReader::ReadString(int16 length, bool autoincrement)
{
	char* str = (char*)calloc(length + 1, sizeof(char));
	memcpy(str, fBuffer + fOffset, length);
	str[length] = '\0';

	if (autoincrement)
		fOffset += length;

	return str;
}


void
BufferReader::Debug(bool fromOffset)
{
	uint16 i = 0, j = 0;
	int breakpoint = 0;
	uchar* buf = Buffer();
	int32 size = Length();

	if (fromOffset) {
		buf += Offset();
		size -= Offset();
	}

	for(; i < size; i++) {
		fprintf(stdout, "%02x ", (uchar)buf[i]);
		breakpoint++;	

		if (!((i + 1) % 16) && i) {
			fprintf(stdout, "\t\t");
			for(j = ((i + 1) - 16); j < ((i + 1) / 16) * 16; j++) {
				if ((buf[j] < 0x21) || (buf[j] > 0x7d))
					fprintf(stdout, ".");
				else
					fprintf(stdout, "%c", (uchar)buf[j]);
			}
			fprintf(stdout, "\n");
			breakpoint = 0;
		}
	}

	if(breakpoint == 16) {
		fprintf(stdout, "\n");
		return;
	}

	for(; breakpoint < 16; breakpoint++)
		fprintf(stdout, "   ");

	fprintf(stdout, "\t\t");

	for (j = size - (size % 16); j < size; j++) {
		if (buf[j] < 30)
			fprintf(stdout, ".");
		else
			fprintf(stdout, "%c", (uchar)buf[j]);
	}

	fprintf(stdout, "\n");
}


//	#pragma mark Operators


bool
BufferReader::operator==(const BufferReader& rhs)
{
	uchar* rhsData = rhs.Buffer();
	int32 rhsLength = rhs.Length();

	return (Length() == rhsLength) &&
		(memcmp(Buffer(), rhsData, rhsLength) == 0);
}


bool
BufferReader::operator==(const BufferReader* rhs)
{
	uchar* rhsData = rhs->Buffer();
	int32 rhsLength = rhs->Length();

	return (Length() == rhsLength) &&
		(memcmp(Buffer(), rhsData, rhsLength) == 0);
}
