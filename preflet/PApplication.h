/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PAPPLICATION_H
#define PAPPLICATION_H

#include <app/Application.h>

class PWindow;

class PApplication : public BApplication
{
	public:
				PApplication();

		virtual bool	QuitRequested();

		virtual void	ReadyToRun();

	private:
		PWindow*	fWindow;
};

#endif // PAPPLICATION_H
