/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include <image.h>

#include "DeskbarIconUtils.h"

extern "C" {

status_t
our_image(image_info& image)
{
	team_id team = 0;
#ifdef __HAIKU__
	team = B_CURRENT_TEAM;
#endif

	int32 cookie = 0;
	while (get_next_image_info(team, &cookie, &image) == B_OK) {
		if ((char *)our_image >= (char *)image.text
			&& (char *)our_image <= (char *)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}

}
