#include "ViewFactory.h"

#include <interface/Box.h>
#include <interface/TextControl.h>
#include <interface/Button.h>
#include <interface/CheckBox.h>
#include <interface/StringView.h>

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
	view = new BView(name, flags & B_SUPPORTS_LAYOUT);
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
BCheckBox *ViewFactory::Create<BCheckBox>(BRect rect, const char* name, const char* label, BMessage* msg) {
	BCheckBox *checkbox = NULL;
#ifdef __HAIKU__
	checkbox = new BCheckBox(name, label, msg);
#else
	checkbox = new BCheckBox(rect, name, label, msg);
#endif

	return checkbox;
}

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
BTextControl *ViewFactory::Create<BTextControl>(BRect rect, const char *name, const char* label,
	const char* value, BMessage* msg, uint32 resize, uint32 flags) {
	BTextControl *text = NULL;
	
#ifdef __HAIKU__
	text = new BTextControl(name, label, value, msg, flags);
#else
	text = new BTextControl(rect, name, label, value, msg, resize, flags);
#endif

	return text;
};

template <>
BStringView* ViewFactory::Create<BStringView>(BRect rect, const char* name, const char* label, uint32 resize, uint32 flags) {
	BStringView *view = NULL;

#ifdef __HAIKU__
	view = new BStringView(name, label, flags);
#else
	view = new BStringView(rect, name, label, resize, flags);
#endif

	return view;
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
	: BView(name, flags | B_SUPPORTS_LAYOUT) {
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
