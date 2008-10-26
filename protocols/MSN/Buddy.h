#ifndef BUDDY_H
#define BUDDY_H

#include <stdio.h>
#include <String.h>

#include "MSNConstants.h"
#include "MSNObject.h"

class Buddy {
	public:
					Buddy(const char *passport);
					~Buddy();

		const char	*Passport(void);
			void	Passport(const char *passport);
		const char	*FriendlyName(void);
			void	FriendlyName(const char *friendly);
			int8	Status(void);
			void	Status(int8 status);
			int32	Capabilities(void);
			void	Capabilities(int32 caps);
		MSNObject	*DisplayPicture(void);
			void	DisplayPicture(MSNObject *obj);
			int32	Lists(void);
			void	Lists(int32 lists);
	
	private:
		BString		fPassport;
		BString		fFriendly;
		int8		fStatus;
		int32		fCaps;
		int32		fLists;
		MSNObject	*fDisplay;
};

#endif
