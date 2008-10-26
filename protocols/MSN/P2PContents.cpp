#include "P2PContents.h"

char *kFieldOrder[] = {
	"EUF-GUID",
	"SessionID",
	"AppID",
	"Context",
	NULL
};

P2PContents::P2PContents(void) {
	fDirty = true;
};

P2PContents::~P2PContents(void) {
};
		
void P2PContents::AddField(const char *field, const char *contents, int32 length = -1) {
	fDirty = true;
	if (length == -1) length = strlen(contents);
	
	BMallocIO * value = new BMallocIO();
	value->Write(contents, length);
	
	fFields[field] = value;
};

int32 P2PContents::Fields(void) {
	return fFields.size();
};

int32 P2PContents::FieldAt(int32 index, char *contents) {
};

void P2PContents::AppendContent(char *content, int32 length = -1) {
	fDirty = true;
	if (length == -1) length = strlen(content);
	
	fContents.Write(content, length);
};

char *P2PContents::Content(void) {
	return (char *)fContents.Buffer();
};

int32 P2PContents::ContentLength(void) {
	return fContents.BufferLength();
};

char *P2PContents::Flatten(void) {
	if (fDirty) {
		fFlattened.SetSize(0);
		fFlattened.Seek(0, SEEK_SET);

		fieldmap::iterator fIt;
		int index = 0;
		char *name = kFieldOrder[index];

		while ((name = kFieldOrder[index++]) != NULL) {
			fIt = fFields.find(name);
			if (fIt != fFields.end()) {
				fFlattened.Write(fIt->first.String(), fIt->first.Length());
				fFlattened.Write(": ", strlen(": "));
				fFlattened.Write((char *)fIt->second->Buffer(), fIt->second->BufferLength());
				fFlattened.Write("\r\n", strlen("\r\n"));
			};
		};
	
		for (fIt = fFields.begin(); fIt != fFields.end(); fIt++) {
			bool alreadyUsed = false;
			index = 0;
			while ((name = kFieldOrder[index++]) != NULL) {
				if (fIt->first == name) {
					alreadyUsed = true;
					break;
				};
			};
			if (alreadyUsed == false) {
				fFlattened.Write(fIt->first.String(), fIt->first.Length());
				fFlattened.Write(": ", strlen(": "));
				fFlattened.Write((char *)fIt->second->Buffer(), fIt->second->BufferLength());
				fFlattened.Write("\r\n", strlen("\r\n"));
			};
		};
		
//		fieldmap::iterator fIt;
//		for (fIt = fFields.begin(); fIt != fFields.end(); fIt++) {
//			fFlattened.Write(fIt->first.String(), fIt->first.Length());
//			fFlattened.Write(": ", strlen(": "));
//			fFlattened.Write(fIt->second->Buffer(), fIt->second->BufferLength());
//			fFlattened.Write("\r\n", strlen("\r\n"));
//		};
		
		fFlattened.Write(fContents.Buffer(), fContents.BufferLength());
		fFlattened.Write("\r\n", strlen("\r\n"));
//		int8 null = 0;
		char null = '\0';
		fFlattened.Write((char *)&null, sizeof(null));
		
		fDirty = false;
	};
	
	return (char *)fFlattened.Buffer();
};

int32 P2PContents::FlattenedLength(void) {
	if (fDirty) Flatten();
	
	return fFlattened.BufferLength();
};

void P2PContents::Debug(void) {
	char *buff = Flatten();
	int32 len = FlattenedLength();
	
	for (int32 i = 0; i < len; i++) {
		if ((buff[i] == '\n') || (buff[i] == '\r'))
			printf("%c", buff[i]);
		else if ((buff[i] < 0x20) || (buff[i] > 0x7e))
			printf("0x%x", (unsigned char)buff[i]);
		else
			printf("%c", buff[i]);
	};
};
