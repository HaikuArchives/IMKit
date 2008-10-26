#ifndef P2PCONTENTS_H
#define P2PCONTENTS_H

#include <stdio.h>
#include <String.h>
#include <DataIO.h>

#include <map>

typedef map<BString, BMallocIO*> fieldmap;

class P2PContents {
	public:
					P2PContents(void);
					~P2PContents(void);
		
			void	AddField(const char *field, const char *contents, int32 length = -1);
			int32	Fields(void);
			int32	FieldAt(int32 index, char *contents);

			void	AppendContent(char *content, int32 length = -1);
			char	*Content(void);
			int32	ContentLength();
			
			char	*Flatten(void);
			int32	FlattenedLength(void);

			void	Debug(void);

	private:
		fieldmap	fFields;
		BMallocIO	fContents;
		BMallocIO	fFlattened;
		bool		fDirty;
			
};

#endif
