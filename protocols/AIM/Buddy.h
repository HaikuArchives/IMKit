#ifndef BUDDY_H
#define BUDDY_H

#include <String.h>

class Buddy {
	public:
					Buddy(void);
					Buddy(const char *name, uint16 group, uint16 item);
	
		uint16		GroupID(void);
		uint16		ItemID(void);
		const char	*Name(void);
	
	private:
		uint16		fGroupID;
		uint16		fItemID;
		uchar		fIconHash[16];
		
		BString		fName;
};

#endif
