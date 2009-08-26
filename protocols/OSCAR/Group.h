#ifndef GROUP_H
#define GROUP_H

#include <String.h>

#include <vector>

typedef std::vector<int16> child_t;

class Group {
	public:
							Group(int id, const char *name);
							~Group(void);
	
		int16				Id(void);
		const char 			*Name(void);
		
		int16				ItemsInGroup(void);
		int16				ItemAt(int16 index);
		void				AddItem(int16 item);
			
	private:
		int16				fId;
		BString				fName;
		child_t				fChildren;
};

#endif
