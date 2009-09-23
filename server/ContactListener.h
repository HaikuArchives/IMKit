/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_LISTENER_H
#define _CONTACT_LISTENER_H

#include "ConnectionStore.h"

namespace IM {
	class ContactListener {
	public:
		virtual void ContactAdded(Contact* contact) {}
		virtual void ContactModified(Contact* contact,
			ConnectionStore* oldConnections, ConnectionStore* newConnections) {}
		virtual	void ContactRemoved(Contact* contact,
			ConnectionStore *oldConnections) {}
	};
};

#endif	// _CONTACT_LISTENER_H
