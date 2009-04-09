#ifndef CONTACTHANDLE_H
#define CONTACTHANDLE_H

#include <Entry.h>

class ContactHandle {
	public:
								ContactHandle(void);
								ContactHandle(const ContactHandle &c);
								
			bool				operator < (const ContactHandle &c) const;
			bool				operator == (const ContactHandle &c) const;
			
			ino_t				node;
			entry_ref			entry;
};

#endif // CONTACTHANDLE_H
