#ifndef TLV_H
#define TLV_H

#include <be/support/SupportDefs.h>

#include <list>

class TLV {
	public:
						TLV(uint16 type);
						TLV(uint16 type, const char *value, uint16 length);
						TLV(const uchar *buffer, int16 bufferLen);
						TLV(void);
						~TLV(void);
						
			status_t	Value(const char *value, uint16 length);
			const char	*Value(void);
			uint16		Length(void);
			uint16		Type(void);
			void		Type(uint16 type);
			
			const char	*Flatten(void);
			uint16		FlattenedSize(void);
			
			status_t	AddTLV(TLV *data);
		
	private:
		uint16			fType;
		uint16			fLength;
		char			*fValue;
		char			*fFlatten;
		uint16			fFlattenedSize;
		bool			fDirty;
		list<TLV *>		fTLVs;
};

#endif
