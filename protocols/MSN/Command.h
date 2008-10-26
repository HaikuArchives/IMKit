#ifndef COMMAND_H
#define COMMAND_H

#include <String.h>
#include <DataIO.h>

#include <vector>
#include <map.h>

typedef vector<BMallocIO*> payloadv;

/**
	Class to manage encoding and decoding of MSN commands.
	
	Decoding:
		Command cmd("");
		cmd.MakeObject( myStringFromMSN );
		...
		
	Encoding:
		Command cmd("CMD");
		cmd.AddParam("123");
		cmd.AddPayload(myData, myDataSize);
		
		const char * toNetwork = cmd.Flatten( nextTrID );
		
		send_to_network( toNetwork, cmd.FlattenedSize() );
*/
class Command {
	public:
					Command(const char *type);
					~Command(void);
				
		const char *TypeStr(void) { return fType.String(); };
		BString		Type(void) { return fType; };
		status_t	AddParam(const char *param, bool encode = false);
		const char	*Param(int32 index, bool decode = false);
		int32		Params(void) { return fParams.size(); };
		
		const char	*Flatten(int32 sequence);
		int32		FlattenedSize(void);
		
		int32		TransactionID(void) { return fTrID; };

		status_t	AddPayload(const char *payload, int32 length = -1, bool encode = true);
		const char	*Payload(int32 index);
		int32		Payloads(void) { return fPayloads.size(); };

		status_t	MakeObject(const char *string);

		bool		UseTrID(void) { return fUseTrID; };
		void		UseTrID(bool use) { fUseTrID = use; };

		bool		ExpectsPayload(int32 *payloadSize);
		
		void		Debug(void);
	private:
		int32		fTrID;
		bool		fUseTrID;
		bool		fDirty;
//		int32		fFlattenedSize;
//		char		*fFlattened;
		BMallocIO	fFlattened;

		BString		fType;
		vector<BString>
					fParams;
		payloadv	fPayloads;

		map<BString, bool>
					gExpectsPayload;
};

#endif
