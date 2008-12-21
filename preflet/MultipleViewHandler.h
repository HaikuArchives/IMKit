/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef MULTIPLEVIEWHANDLER_H
#define MULTIPLEVIEWHANDLER_H

class MultipleViewHandler {
	public:
		virtual void		ShowServerOverview(void) = 0;
		virtual void		ShowProtocolsOverview(void) = 0;
		virtual void		ShowClientsOverview(void) = 0;
};

#endif // MULTIPLEVIEWHANDLER_H
