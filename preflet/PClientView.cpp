/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 */

#include "PClientView.h"
#include "PUtils.h"

PClientView::PClientView(BRect frame, const char *name, const char *title)
	: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS) {
};

//#pragma mark SettingsController Hooks

status_t PClientView::Init(SettingsHost *host) {
	fHost = host;
	return B_OK;
};

status_t PClientView::Save(BView *view, const BMessage *tmplate, BMessage *settings) {
	return SaveSettings(view, *tmplate, settings);
};

status_t PClientView::Revert(BView *view, const BMessage *tmplate) {
	return B_OK;
};
