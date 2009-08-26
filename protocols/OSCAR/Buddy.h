#ifndef BUDDY_H
#define BUDDY_H

#include <String.h>

#include <vector>

class BufferReader;

typedef std::vector <BufferReader *> capability_t;
typedef std::vector<uint16> groupid_t;

class Buddy {
	public:
							Buddy(void);
							Buddy(const char *name, uint16 item);
							~Buddy(void);
	
		uint16				ItemID(void);
		const char			*Name(void);

		int32				CountGroups(void);
		uint16				GroupAt(int32 index);
		void				AddGroup(uint16 group);
		void				ClearGroups(void);
		bool				IsInGroup(uint16 group);
		
		void				SetUserclass(uint16 userclass);
		uint16				Userclass(void);
		
		bool				HasCapability(const char *capability, int32 len);
		void				AddCapability(const char *capability, int32 len);
		void				ClearCapabilities(void);
	
		bool				IsMobileUser(void);
	
	private:
		uint16				fItemID;
		uchar				fIconHash[16];
		
		BString				fName;
		capability_t		fCapabilities;
		uint16				fUserclass;
		groupid_t			fGroups;
};

#endif
