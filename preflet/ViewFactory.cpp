#include "ViewFactory.h"

#include <interface/Box.h>
#include <interface/TextControl.h>
#include <interface/Button.h>

#include "common/NotifyingTextView.h"

//#pragma mark Public Methods

template <class T>
T *ViewFactory::Create(BRect rect, const char *name, uint32 resize, uint32 flags) {
	return NULL;
};

template <>
BView *ViewFactory::Create<BView>(BRect rect, const char *name, uint32 resize, uint32 flags) {
	BView *view = NULL;

#ifdef __HAIKU__
	view = new BView(name, flags);
#else
	view = new BView(rect, name, resize, flags);
#endif

#if B_BEOS_VERSION > B_BEOS_VERSION_5
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	view->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	view->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	view->SetHighColor(0, 0, 0, 0);
#endif

	return view;
};

template <>
BButton *ViewFactory::Create<BButton>(BRect rect, const char* name, const char* label, BMessage* msg) {
	BButton *button = NULL;
#ifdef __HAIKU__
	button = new BButton(name, label, msg);
#else
	button = new BButton(rect, name, label, msg);
#endif

	return button;
};

template <>
BBox *ViewFactory::Create<BBox>(BRect rect, const char *name, uint32 resize, uint32 flags) {
	BBox *box = NULL;
#ifdef __HAIKU__
	box = new BBox(name);
#else
	box = new BBox(rect, name, resize, flags);
#endif

	return box;
};

template <>
BTextControl *ViewFactory::Create<BTextControl>(BRect rect, const char *name, uint32 resize, uint32 flags) {
	BTextControl *text = NULL;
	
#ifdef __HAIKU__
	text = new BTextControl(name, name, NULL, NULL);
#else
	text = new BTextControl(rect, name, name, NULL, NULL);
#endif

	return text;
};

template <>
NotifyingTextView *ViewFactory::Create<NotifyingTextView>(BRect rect, const char *name, uint32 resize, uint32 flags) {
	NotifyingTextView *view = NULL;

#ifdef __HAIKU__
	view = new NotifyingTextView(name, flags);
#else
	view = new NotifyingTextView(rect, name, resize, flags);
#endif

	return view;
};

AbstractView::AbstractView(BRect frame, const char *name, uint32 resizeMask, uint32 flags)
#ifdef __HAIKU__
	: BView(name, flags) {
#else
	: BView(frame, name, resizeMask, flags) {
#endif
};

void AbstractView::AttachedToWindow(void) {
#if B_BEOS_VERSION > B_BEOS_VERSION_5
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
#else
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 0);
#endif
};
