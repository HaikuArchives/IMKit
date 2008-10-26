/*
 * Copyright 2008, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef IM_RESOURCES_H
#define IM_RESOURCES_H

#ifdef HAIKU_TARGET_PLATFORM_HAIKU

enum {
	kAvailableStatusIcon		= 1000,
	kAwayStatusIcon			= 1001,
	kBlockStatusIcon		= 1002,
	kOfflineStatusIcon		= 1003,

	kEmailIcon			= 2000,
	kPersonIcon			= 2001,
};

#else // HAIKU_TARGET_PLATFORM_HAIKU

enum {
	kAvailableStatusIconLarge	= 1000,
	kAvailableStatusIconSmall	= 1001,
	kAwayStatusIconSmall		= 1002,
	kAwayStatusIconLarge		= 1003,
	kBlockStatusIconSmall		= 1004,
	kBlockStatusIconLarge		= 1005,
	kOfflineStatusIconSmall		= 1006,
	kOfflineStatusIconLarge		= 1007,

	kEmailIconSmall			= 2000,
	kEmailIconLarge			= 2001,
	kPeopleIconLarge		= 2002,
	kPeopleIconSmall		= 2003,
};

#endif // HAIKU_TARGET_PLATFORM_HAIKU

enum {
	kDeskbarOnlineIcon		= 3000,
	kDeskbarOfflineIcon		= 3001,
	kDeskbarAwayIcon		= 3002,
	kDeskbarGenericIcon		= 3003,
};

#endif // IM_RESOURCES_H
