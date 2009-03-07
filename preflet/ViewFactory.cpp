#include "ViewFactory.h"

#include <interface/Box.h>
#include <interface/TextControl.h>
#include <interface/View.h>

//#pragma mark Public Methods

template <class T>
T *ViewFactory::Create(BRect rect, const char *name, uint32 resize, uint32 flags) {
	return NULL;
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