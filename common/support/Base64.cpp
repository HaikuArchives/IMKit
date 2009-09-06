/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 */

#include <stdlib.h>

#include "Base64.h"

//	#pragma mark Constants

const char b64_table[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};


//	#pragma mark Functions


char*
Base64Encode(const char* in, int32 length)
{
	unsigned long concat;
	int32 i = 0, k = 0;
	char* out = (char*)calloc((int)ceil(length * 1.33) + 4, sizeof(char));

	// --4 is a safety extension, designed to cause retirement *before* it actually gets too long
	int32 curr_linelength = 4;

	while (i < length) {
		concat = ((in[i] & 0xff) << 16);

		if ((i+1) < length)
			concat |= ((in[i+1] & 0xff) << 8);
		if ((i+2) < length)
			concat |= (in[i+2] & 0xff);

		i += 3;

		out[k++] = b64_table[(concat >> 18) & 63];
		out[k++] = b64_table[(concat >> 12) & 63];
		out[k++] = b64_table[(concat >> 6) & 63];
		out[k++] = b64_table[concat & 63];

		if (i >= length) {
			for (int32 v = 0; v <= (i - length); v++)
				out[k-v] = '=';
		}

		curr_linelength += 4;
	}

	out[k] = '\0';
	return out;
}


int32
Base64Decode(const char* in, int32 length, uchar** result)
{
	int32 resultLen = 0;
	uchar* target = *result;

	target = (uchar*)calloc(length * 5, sizeof(uchar));

	unsigned long concat = 0;
	unsigned long value = 0;
	int32 lastOutLine = 0;

	for (int32 i = 0; i < length; i += 4) {
		int32 j;
		concat = 0;

		for (j = 0; j < 4 && (i + j) < length; j++) {
			value = in[i + j];

			if (value == '\n' || value == '\r') {
				// Jump over line breaks
				lastOutLine = resultLen;
				i++;
				j--;
				continue;
			}

			if ((value >= 'A') && (value <= 'Z'))
				value -= 'A';
			else if ((value >= 'a') && (value <= 'z'))
				value = value - 'a' + 26;
			else if ((value >= '0') && (value <= '9'))
				value = value - '0' + 52;
			else if (value == '+')
				value = 62;
			else if (value == '/')
				value = 63;
			else if (value == '=')
				break;
			else {
				// There is an invalid character in this line - we will
				// ignore the whole line and go to the next
				resultLen = lastOutLine;
				while (i < length && in[i] != '\n' && in[i] != '\r')
					i++;
				concat = 0;
			}

			value = value << ((3-j)*6);

			concat |= value;
		}

		if (j > 1)
			target[resultLen++] = (concat & 0x00ff0000) >> 16;
		if (j > 2)
			target[resultLen++] = (concat & 0x0000ff00) >> 8;
		if (j > 3)
			target[resultLen++] = (concat & 0x000000ff);
	}

	*result = (uchar *)realloc(target, sizeof(uchar) * resultLen);
	return resultLen;
}
