/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */

#ifndef CONNECTIONSTORE_H
#define CONNECTIONSTORE_H

#include <libim/Connection.h>

#include "common/GenericStore.h"

namespace IM {

	class ConnectionStore : public IM::GenericListStore<IM::Connection> {
	};

};

#endif // CONNECTIONSTORE_H
