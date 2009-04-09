/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
#ifndef CONTACT_LISTENER_H
#define CONTACT_LISTENER_H

#include "common/GenericStore.h"

namespace IM {
	class Contact;
	class ConnectionStore;

	class ContactListener {
		public:
			virtual void		ContactAdded(Contact *contact) {};
			virtual void		ContactModified(Contact *contact, ConnectionStore *oldConnections, ConnectionStore *newConnections) {};
			virtual void		ContactRemoved(Contact *contact, ConnectionStore *oldConnections) {};
	};
};

#endif // CONTACT_LISTENER_H
