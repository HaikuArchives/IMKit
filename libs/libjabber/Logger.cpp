/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <StorageDefs.h>

#include "Logger.h"


void
logmsg(const char* message, ...)
{
	va_list varg;
	char buffer[2048];

	va_start(varg, message);
	vsprintf(buffer, message, varg);

	char timestr[64];
	char today[11];
	time_t now = time(NULL);
	strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M", localtime(&now));
	timestr[63] = '\0';
	strftime(today, sizeof(today), "%Y-%m-%d", localtime(&now));
	today[10] = '\0';

	char filename[B_PATH_NAME_LENGTH];
	snprintf(filename, B_PATH_NAME_LENGTH, "/boot/var/tmp/jabber-%s.log", today);
	FILE* file = fopen(filename, "a+");
	fprintf(file, "%s %s\n", timestr, buffer);
	fclose(file);
}
