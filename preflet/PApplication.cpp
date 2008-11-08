/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		Mikael Eiman <m_eiman@eiman.tv>
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include "PApplication.h"
#include "PWindow.h"
#include "PResources.h"

PApplication::PApplication(void)
	: BApplication(PREFLET_SIGNATURE)
{
}


void
PApplication::ReadyToRun()
{
	fWindow = new PWindow();
	fWindow->Show();
}


bool
PApplication::QuitRequested()
{
	return true;
}
