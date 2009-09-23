/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_HANDLE_H
#define _CONTACT_HANDLE_H

#include <Entry.h>

class ContactHandle {
public:
					ContactHandle();
					ContactHandle(const ContactHandle& c);

	bool			operator<(const ContactHandle& c) const;
	bool			operator==(const ContactHandle& c) const;

	ino_t			node;
	entry_ref		entry;
};

#endif	// _CONTACT_HANDLE_H
