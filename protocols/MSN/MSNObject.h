#ifndef MSNOBJECT_H
#define MSNOBJECT_H

#include <vector>
#include <String.h>
#include <map.h>

#include <File.h>
#include <openssl/sha.h>

class MSNObject {
	public:
					MSNObject(const char *path, const char *username, int32 type = 3);
					MSNObject(const char *text, int32 length);
					~MSNObject(void);

			int32	Length();
		const char	*Value();

		const char	*Base64Encoded();
			int32	Base64Length();
		
		const char	*Creator(void);
			int32	Size(void);
			int32	Type(void);
		const char	*Location(void);
		const char 	*Friendly(void);
		const char	*SHA1D(void);
		const char	*SHA1C(void);

	private:
//		int32		Base64Encode(const char *raw, int32 length, char *encoded);
		char 		*Base64Encode(const char *in, off_t length);
		BString		fContents;
		BString		fBase64;
		
		BString		fCreator;
		int32		fSize;
		int32		fType;
		BString		fLocation;
		BString		fFriendly;
		BString		fSHA1D;
		BString		fSHA1C;
};

#endif
