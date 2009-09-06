#ifndef IMKIT_BASE64_H
#define IMKIT_BASE64_H

#include <Message.h>

char *Base64Encode(const char *in, int32 length);
int32 Base64Decode(const char *in, int32 length, uchar **result);

#endif
