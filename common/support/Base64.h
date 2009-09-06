/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _BASE64_H
#define _BASE64_H

#include <Message.h>

char* Base64Encode(const char* in, int32 length);
int32 Base64Decode(const char* in, int32 length, uchar** result);

#endif	// _BASE64_H
