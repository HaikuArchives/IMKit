/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PWINDOW_H
#define PWINDOW_H

#include <interface/Window.h>

class PView;

class PWindow : public BWindow {
	public:
							PWindow(void);

		// BWindow Hooks
		virtual bool		QuitRequested(void);

	private:
		PView				*fView;
};

#endif // PWINDOW_H
