/*
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 */
#ifndef _CONNECTION_STORE_H
#define _CONNECTION_STORE_H

#include <libim/Connection.h>

#include "common/GenericStore.h"

namespace IM {
	class ConnectionStore : public IM::GenericListStore<IM::Connection> {
	};
};

#endif	// _CONNECTION_STORE_H
