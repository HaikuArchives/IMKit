/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <libim/Manager.h>

#include "Misc.h"


status_t
SendMessageToServer(BMessage* msg)
{
	IM::Manager manager;
	return manager.SendMessage(msg);
}
