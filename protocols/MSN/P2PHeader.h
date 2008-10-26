#ifndef P2PHEADER_H
#define P2PHEADER_H

#include <vector>
#include <String.h>
#include <map.h>
#include <support/ByteOrder.h>

#include "P2PContents.h"

extern const char *kMSNLPVer;

typedef map<BString, BMallocIO*> fieldmap;

class P2PHeader {
	public:
			enum {
				p2pFlagsNone = 0,
				p2pFlagsReply = 0x02,
				p2pFlagsError = 0x08,
				p2pFlagsDataDPEmoticon = 0x20,
				p2pFlagsFile = 0x01000030,
			};
	
						P2PHeader(const char *method, const char *recipient);
						~P2PHeader(void);

			void		AddField(char *field, char *contents, int32 length = -1);
			int32		Fields(void);
			int32		FieldAt(int32 index, char *contents);

			void		Method(char *method);
		const char		*Method(void);
			void		Recipient(char *recipient);
		const char		*Recipient(void);

			void		Content(P2PContents *content);
			P2PContents	*Content(void);

			int32		FlattenedLength(void);
		const char		*Flatten(void);

//			Headers
			void		SessionID(int32 session);
			int32		SessionID(void);
			void		Identifier(int32 identifier);
			int32		Identifier(void);
			void		Offset(int64 offset);
			int64		Offset(void);
			int64		Size(void);
			int32		MessageSize(void);
			void		Flags(int32 flags);
			int32		Flags(void);
			void		AckSessionID(int32 ack);
			int32		AckSessionID(void);
			void		AckUniqueID(int32 ack);
			int32		AckUniqueID(void);
			void		AckDataSize(int64 ack);
			int64		AckDataSize(void);
			
			void		Debug(void);

	private:
			BString		fMethod;
			BString		fRecipient;
	
			fieldmap	fFields;

			int32		fSessionID;
			int32		fIdentifier;
			int64		fOffset;
			int64		fSize;
			int32		fMessageSize;
			int32		fFlags;
			int32		fAckSessID;
			int32		fAckUniqID;
			int64		fAckDataSize;
			
			P2PContents	*fContent;
			
			bool		fDirty;
			BMallocIO	fFlattened;
};

#endif
