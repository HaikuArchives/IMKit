#include "P2PHeader.h"

const char *kMSNLPVer = " MSNLP/1.0\r\n";

char *kHeaderOrder[] = {
	"To",
	"From",
	"Via",
	"CSeq",
	"Call-ID",
	"Max-Forwards",
	"Content-Type",
	"Content-Length",
	NULL
};

P2PHeader::P2PHeader(const char *method, const char *recipient) {
	fMethod = method;
	fMethod.ToUpper();
	fRecipient = recipient;

	fContent = NULL;
	fDirty = true;

	fSessionID = 0;
	fIdentifier = 0;
	fOffset = 0;
	fSize = 0;
	fMessageSize = 0;
	fFlags = 0;
	fAckSessID = 0;
	fAckUniqID = 0;
	fAckDataSize = 0;
};

P2PHeader::~P2PHeader(void) {
	if (fContent) delete fContent;
};

void P2PHeader::AddField(char *field, char *contents, int32 length = -1) {
	fDirty = true;

	if (length == -1) length = strlen(contents);
	BMallocIO * data = new BMallocIO();
	data->Write(contents, length);
	
	fFields[field] = data;
};

int32 P2PHeader::Fields(void) {
	return fFields.size();
};

int32 P2PHeader::FieldAt(int32 index, char *contents) {
	return -1;
};

void P2PHeader::Method(char *method) {
	fDirty = true;

	fMethod = method;
	fMethod.ToUpper();
};

const char *P2PHeader::Method(void) {
	return fMethod.String();
};

void P2PHeader::Recipient(char *recipient) {
	fDirty = true;
	
	fRecipient = recipient;
};

const char *P2PHeader::Recipient(void) {
	return fRecipient.String();
};

void P2PHeader::Content(P2PContents *content) {
	fDirty = true;

	if (fContent) delete fContent;
	fContent = content;
};

P2PContents *P2PHeader::Content(void) {
	return fContent;
};

int32 P2PHeader::FlattenedLength(void) {
	if (fDirty) Flatten();
	
	return fFlattened.BufferLength();
};

const char *P2PHeader::Flatten(void) {
	if (fDirty) {
		fFlattened.SetSize(0);
		fFlattened.Seek(SEEK_SET, 0);
		fFlattened.Write((char *)&fSessionID, sizeof(fSessionID));
		fFlattened.Write((char *)&fIdentifier, sizeof(fIdentifier));
		fFlattened.Write((char *)&fOffset, sizeof(fOffset));
		fFlattened.Write((char *)&fSize, sizeof(fSize));
		fFlattened.Write((char *)&fMessageSize, sizeof(fMessageSize));
		fFlattened.Write((char *)&fFlags, sizeof(fFlags));
		fFlattened.Write((char *)&fAckSessID, sizeof(fAckSessID));
		fFlattened.Write((char *)&fAckUniqID, sizeof(fAckUniqID));
		fFlattened.Write((char *)&fAckDataSize, sizeof(fAckDataSize));

		fFlattened.Write("\r\n", strlen("\r\n"));
		fFlattened.Write(fMethod.String(), fMethod.Length());
		fFlattened.Write(" MSNMSGR:", strlen(" MSNMSGR:"));
		fFlattened.Write(fRecipient.String(), fRecipient.Length());
		fFlattened.Write(kMSNLPVer, strlen(kMSNLPVer));
	
		fieldmap::iterator fIt;
		int index = 0;
		char *name = kHeaderOrder[index];

//		Oh yes, brilliant! The headers have to be in a certain order! I love this shit!
//		Jimmy, hand me my pistola!
		while ((name = kHeaderOrder[index++]) != NULL) {
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
			while ((name = kHeaderOrder[index++]) != NULL) {
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
	
		if (fContent) {
			BString len = "";
			len << fContent->FlattenedLength() << "\r\n\r\n";
			len.Prepend("Content-Length: ");
			fFlattened.Write(len.String(), len.Length());
			fFlattened.Write(fContent->Flatten(), fContent->FlattenedLength());
			fFlattened.Write("\r\n", strlen("\r\n"));
		} else {
			BString con = "Content-Length: 0\r\n";
			fFlattened.Write(con.String(), con.Length());
		};
	
		int32 offset = sizeof(fSessionID) + sizeof(fIdentifier) + sizeof(fOffset);
		char b[] = { 0x00, 0x00, 0x00, 0x00 };

// Culprint here, fooooooooooooooo!!!!!!!

		fMessageSize = fFlattened.BufferLength() - 48 - 4; // Header;
		fSize = fMessageSize;
		fFlattened.WriteAt(offset, (char *)&fSize, sizeof(fSize));
		fFlattened.WriteAt(offset + sizeof(fSize), b, sizeof(b));

		fFlattened.WriteAt(40, (char *)&fSize, sizeof(fSize));

//		b[3] = 0x01;
		fFlattened.Write(b, 4);
//		int32 appID = 1;
//		fFlattened.Write((char *)&appID, sizeof(appID));
	
		fDirty = false;
	};
	
	return (char *)fFlattened.Buffer();
};

void P2PHeader::SessionID(int32 session) {
	fSessionID = session;
};

int32 P2PHeader::SessionID(void) {
	return fSessionID;
};

void P2PHeader::Identifier(int32 identifier) {
	fIdentifier = identifier;
};

int32 P2PHeader::Identifier(void) {
	return fIdentifier;
};

void P2PHeader::Offset(int64 offset) {
	fOffset = offset;
};

int64 P2PHeader::Offset(void) {
	return fOffset;
};

int64 P2PHeader::Size(void) {
	return fSize;
};

int32 P2PHeader::MessageSize(void) {
	int32 ret = 0;
	if (fContent) ret = fContent->FlattenedLength();
	return ret;
};

void P2PHeader::Flags(int32 flags) {
	fFlags = flags;
};

int32 P2PHeader::Flags(void) {
	return fFlags;
};

void P2PHeader::AckSessionID(int32 ack) {
	fAckSessID = ack;
};

int32 P2PHeader::AckSessionID(void) {
	return fAckSessID;
};

void P2PHeader::AckUniqueID(int32 ack) {
	fAckUniqID = ack;
};

int32 P2PHeader::AckUniqueID(void) {
	return fAckUniqID;
};

void P2PHeader::AckDataSize(int64 ack) {
	fAckDataSize = ack;
};

int64 P2PHeader::AckDataSize(void) {
	return fAckDataSize;
};

void P2PHeader::Debug(void) {
	char *buff = (char *)Flatten();
	int32 len = FlattenedLength();
	
	for (int32 i = 0; i < 48; i++) printf("0x%02x ", (unsigned char)buff[i]);
	
	for (int32 i = 48; i < len; i++) {
		if ((buff[i] == '\n') || (buff[i] == '\r'))
			printf("%c", buff[i]);
		else if ((buff[i] < 0x20) || (buff[i] > 0x7e))
			printf("0x%02x ", (unsigned char)buff[i]);
		else
			printf("%c", buff[i]);
	};
};
