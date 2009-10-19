/*
 * Copyright 2008-2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef SERVER_RESOURCES_H
#define SERVER_RESOURCES_H

#ifdef HAIKU_TARGET_PLATFORM_HAIKU

enum {
	kAvailableStatusIcon	= 1000,
	kAwayStatusIcon			= 1001,
	kBlockStatusIcon		= 1002,
	kOfflineStatusIcon		= 1003,

	kEmailIcon				= 2000,
	kPersonIcon				= 2001,

	kDeskbarOnlineIcon		= 3000,
	kDeskbarOfflineIcon		= 3001,
	kDeskbarAwayIcon		= 3002,
	kDeskbarGenericIcon		= 3003
};

#else // HAIKU_TARGET_PLATFORM_HAIKU

// Icon indexes
enum {
	kAvailableStatusIcon		= 0,
	kAwayStatusIcon				= 1,
	kBlockStatusIcon			= 2,
	kOfflineStatusIcon			= 3
};

enum {
	kAvailableStatusIconLarge	= 1000,
	kAwayStatusIconLarge		= 1001,
	kBlockStatusIconLarge		= 1002,
	kOfflineStatusIconLarge		= 1003,

	kAvailableStatusIconSmall	= 1004,
	kAwayStatusIconSmall		= 1005,
	kBlockStatusIconSmall		= 1006,
	kOfflineStatusIconSmall		= 1007,

	kEmailIconSmall				= 2000,
	kEmailIconLarge				= 2001,
	kPeopleIconLarge			= 2002,
	kPeopleIconSmall			= 2003,

	kDeskbarOnlineIcon		= 3000,
	kDeskbarOfflineIcon		= 3001,
	kDeskbarAwayIcon		= 3002,
	kDeskbarGenericIcon		= 3003
};

#endif // HAIKU_TARGET_PLATFORM_HAIKU

#endif // SERVER_RESOURCES_H
