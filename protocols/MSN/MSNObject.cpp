#include "MSNObject.h"
#include <stdio.h>
#include <math.h>

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

MSNObject::MSNObject(const char *path, const char *username, int32 type = 3) {
	fCreator = username;
	fType = type;
	fFriendly = "AAA=";
	fLocation = "IMKitObject.tmp";
	fBase64 = "";

	BFile file(path, B_READ_ONLY);
	if (file.InitCheck() == B_OK) {
		uchar hash[20];
		char *buffer = NULL;
		char *b64;
		file.GetSize((off_t *)&fSize);
	
		buffer = (char *)calloc(fSize, sizeof(char));	
		file.Read(buffer, fSize);
		file.Unset();

		fContents = "<msnobj Creator=\"";
		fContents << username << "\" Size=\"" << fSize << "\" Type=\"" << fType;
		fContents << "\" Location=\"IMKitObject.tmp\" Friendly=\"AAA=\" ";
		fContents << "SHA1D=\"";		

		SHA1((const uchar *)buffer, fSize, hash);
		b64 = Base64Encode((const char *)hash, 20);
		fContents << b64 << "\" ";
		
		fSHA1D = b64;
		
		free(buffer);
		
		BString sha1cSource = "Creator";
		sha1cSource << username << "Size" << fSize << "Type" << fType;
		sha1cSource << "Location" << fLocation;
		sha1cSource << "Friendly" << fFriendly << "SHA1D" << b64;
		
		free(b64);

		fContents << "SHA1C=\"";
		
		SHA1((const uchar *)sha1cSource.String(), sha1cSource.Length(), hash);
		b64 = Base64Encode((const char *)hash, 20);
		fContents << b64 << "\"/>";
		fSHA1C = b64;

		free(b64);
		
	};	
};

MSNObject::MSNObject(const char *text, int32 length) {
	fContents = text;
	fBase64 = "";
	int32 start = 0;
	int32 end = 0;

	start = fContents.IFindFirst("Creator=\"");
	if (start > B_ERROR) {
		start += strlen("Creator=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) fContents.CopyInto(fCreator, start, end - start);
	};

	start = fContents.IFindFirst("Size=\"");
	if (start > B_ERROR) {
		BString temp = "";
		start += strlen("Size=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) {
			fContents.CopyInto(temp, start, end - start);
			fSize = atol(temp.String());
		} else {
			fSize = -1;
		};
	};
	
	start = fContents.IFindFirst("Type=\"");
	if (start > B_ERROR) {
		BString temp = "";
		start += strlen("Type=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) {
			fContents.CopyInto(temp, start, end - start);
			fType = atol(temp.String());
		} else {
			fType = -1;
		};
	};
	
	start = fContents.IFindFirst("Location=\"");
	if (start > B_ERROR) {
		BString temp = "";
		start += strlen("Location=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) fContents.CopyInto(fLocation, start, end - start);
	};
	
	start = fContents.IFindFirst("Friendly=\"");
	if (start > B_ERROR) {
		BString temp = "";
		start += strlen("Friendly=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) fContents.CopyInto(fFriendly, start, end - start);
	};

	start = fContents.IFindFirst("SHA1D=\"");
	if (start > B_ERROR) {
		BString temp = "";
		start += strlen("SHA1D=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) fContents.CopyInto(fSHA1D, start, end - start);
	};

	start = fContents.IFindFirst("SHA1C=\"");
	if (start > B_ERROR) {
		BString temp = "";
		start += strlen("SHA1C=\"");
		end = fContents.FindFirst("\"", start);
		if (end > B_ERROR) fContents.CopyInto(fSHA1C, start, end - start);
	};
};

// Gratuitously *YOINK*ed from the MDR project. Please don't steal my pants as
// recompense.

char *MSNObject::Base64Encode(const char *in, off_t length) {
	unsigned long concat;
	int i = 0;
	int k = 0;
	int curr_linelength = 4; //--4 is a safety extension, designed to cause retirement *before* it actually gets too long
	char *out = (char *)calloc((int)ceil(length * 1.33) + 4, sizeof(char));

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
			int v;
			for (v = 0; v <= (i - length); v++)
				out[k-v] = '=';
		}

		curr_linelength += 4;
	}
	
	out[k] = '\0';

	return out;
}

MSNObject::~MSNObject(void) {
}

int32 MSNObject::Length(void) {
	return fContents.Length();
};

const char *MSNObject::Value(void) {
	return fContents.String();
};

const char *MSNObject::Creator(void) {
	return fCreator.String();
};

int32 MSNObject::Size(void) {
	return fSize;
};

int32 MSNObject::Type(void) {
	return fType;
};

const char *MSNObject::Location(void) {
	return fLocation.String();
};

const char *MSNObject::Friendly(void) {
	return fFriendly.String();
};

const char *MSNObject::SHA1D(void) {
	return fSHA1D.String();
};

const char *MSNObject::SHA1C(void) {
	return fSHA1C.String();
};

const char *MSNObject::Base64Encoded(void) {
	if (fBase64.Length() == 0) {
		BString temp = Value();
		temp << '\0';
	
		char *b64 = Base64Encode(temp.String(), temp.Length());
		fBase64 = b64;
		
		free(b64);
		
	};
	
	return fBase64.String();
};

int32 MSNObject::Base64Length(void) {
	if (fBase64.Length() == 0) Base64Encoded();
	
	return fBase64.Length();
};
