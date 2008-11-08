/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PWINDOW_H
#define PWINDOW_H

#include <interface/Window.h>

class PWindow : public BWindow
{
	public:
				PWindow();

		virtual bool	QuitRequested();

	private:
		void		CenterWindowOnScreen();
};

#endif // PWINDOW_H
