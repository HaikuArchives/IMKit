/*
 * Copyright 2003-2008, IM Kit Team.
 * Distributed under the terms of the MIT License.
 */
#ifndef PUTILS_H
#define PUTILS_H

#include <BeBuild.h>

void CenterWindowOnScreen(BWindow* window);
float BuildGUI(BMessage templ, BMessage settings, const char* viewName, BView* view, bool heading = true);
status_t SaveSettings(BView *panel, BMessage tmplate, BMessage *settings);

#endif // PUTILS_H
