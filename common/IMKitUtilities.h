#ifndef IMUTILITIES_H
#define IMUTILITIES_H

#include <interface/Bitmap.h>
#include <kernel/fs_attr.h>
#include <storage/Node.h>
#include <storage/Path.h>
#include <storage/Entry.h>
#include <storage/Mime.h>

#include <stdlib.h>

#define BEOS_MINI_ICON_ATTRIBUTE 	"BEOS:M:STD_ICON"
#define BEOS_LARGE_ICON_ATTRIBUTE	"BEOS:L:STD_ICON"
#define BEOS_SVG_ICON_ATTRIBUTE		"BEOS:V:STD_ICON"
#define BEOS_SVG_EXTRA_ATTRIBUTE	"BEOS:D:STD_ICON"
#define BEOS_ICON_ATTRIBUTE		"BEOS:ICON"

#define BEOS_SVG_ICON_ATTRIBUTE_TYPE 0x7A49434F
#define BEOS_SVG_EXTRA_ATTRIBUTE_TYPE 0x6949434F

extern const int32 kSmallIcon;
extern const int32 kLargeIcon;

class BResources;

BBitmap *GetTrackerIcon(BNode &, unsigned long, long *);

BBitmap *GetBitmapFromAttribute(const char *name, const char *attribute,
	type_code type = 'BBMP', bool followSymlink = true);

BBitmap* GetIconFromResources(BResources* resources, int32 num, icon_size size);

char *ReadAttribute(BNode node, const char *attribute, int32 *length = NULL);
status_t WriteAttribute(BNode node, const char *attribute, const char *value,
	size_t length, type_code type);


// This will return the standard icon for R5 and the SVG icon for Zeta
BBitmap *ReadNodeIcon(const char *name, int32 size = kSmallIcon,
	bool followSymlink = true);

extern BBitmap * rescale_bitmap( const BBitmap * source, int32 width, int32 height = -1 );

extern "C" status_t our_image(image_info& image);

#endif
