#include "IMKitUtilities.h"

#include <stdio.h>
#include <string.h>

#include <storage/NodeInfo.h>
#include <storage/Resources.h>

#ifdef __HAIKU__
#	include <IconUtils.h>
#else
#	include <storage/Mime.h>
#endif

#ifdef B_ZETA_VERSION
#include <sys_apps/Tracker/Icons.h>
#endif

//#pragma mark Constants

const int32 kSmallIcon = 16;
const int32 kLargeIcon = 32;

//#pragma mark Functions

// Loads the icon. Callers responsibility to free BBitmap

BBitmap *ReadNodeIcon(const char *name, int32 size,
	bool followSymlink) {
	
	BEntry entry(name, followSymlink);
	entry_ref ref;
	BPath path;
	BBitmap *ret = NULL;

	entry.GetRef(&ref);

#ifdef ZETA
 #ifndef B_ZETA_VERSION
	// Zeta RC2 or early code.
	BNode node(BPath(&ref).Path());

	ret = GetTrackerIcon(node, size, NULL);
 #else
	// Zeta RC3 or later.
	ret = new BBitmap(GetTrackerIcon(entry, size));
 #endif
#endif

#ifdef HAIKU
	BNode node(BPath(&ref).Path());

	ret = new BBitmap(BRect(0, 0, size - 1, size - 1), B_RGBA32);
	if (BIconUtils::GetIcon(&node, BEOS_ICON_ATTRIBUTE, BEOS_MINI_ICON_ATTRIBUTE,
		BEOS_LARGE_ICON_ATTRIBUTE, (icon_size)size, ret) < B_OK) {
		delete ret;
		ret = NULL;
	}
#endif

#ifdef BEOS
	BNode node(BPath(&ref).Path());

	ret = new BBitmap(BRect(0,0,size-1,size-1), B_CMAP8);
	if (BNodeInfo::GetTrackerIcon((const entry_ref *)&ref, ret, (icon_size)size) < B_OK) {
		delete ret;
		ret = NULL;
	}
#endif

	return ret;
};

// Loads 'attribute' of 'type' from file 'name'. Returns a BBitmap (Callers 
//  responsibility to delete it) on success, NULL on failure. 

BBitmap *GetBitmapFromAttribute(const char *name, const char *attribute, 
	type_code type, bool followSymlink) {

	BEntry entry(name, followSymlink);
	entry_ref ref;
	BPath path;
	
	entry.GetRef(&ref);

	BBitmap 	*bitmap = NULL;
	size_t 		len = 0;

	if ((name == NULL) || (attribute == NULL)) return NULL;

	BNode node(BPath(&ref).Path());
	
	if (node.InitCheck() != B_OK) {
		return NULL;
	};
	
	attr_info info;
		
	if (node.GetAttrInfo(attribute, &info) != B_OK) {
		node.Unset();
		return NULL;
	};
		
	char *data = (char *)calloc(info.size, sizeof(char));
	len = (size_t)info.size;
		
	if (node.ReadAttr(attribute, type, 0, data, len) != (int32)len) {
		node.Unset();
		free(data);
	
		return NULL;
	};
	
//	Icon is a square, so it's right / bottom co-ords are the root of the bitmap length
//	Offset is 0
	BRect bound = BRect(0, 0, 0, 0);
	bound.right = sqrt(len) - 1;
	bound.bottom = bound.right;
	
	bitmap = new BBitmap(bound, B_COLOR_8_BIT);
	bitmap->SetBits(data, len, 0, B_COLOR_8_BIT);

//	make sure it's ok
	if(bitmap->InitCheck() != B_OK) {
		free(data);
		delete bitmap;
		return NULL;
	};

	return bitmap;
}

BBitmap *GetIconFromResources(BResources *resources, int32 num, icon_size size) {
	if (resources == NULL)
		return NULL;

	size_t nbytes = 0;
	const void* data = NULL;
	type_code type;
	color_space cspace;

#ifdef __HAIKU__
	// First, try to fetch a vector icon on Haiku
	type = B_VECTOR_ICON_TYPE;
	cspace = B_RGBA32;
	data = resources->LoadResource(type, num, &nbytes);
#endif

	if (data == NULL) {
#ifdef __HAIKU__
		// Determine resource type from icon size
		switch (size) {
			case B_MINI_ICON:
				type = B_MINI_ICON_TYPE;
				break;
			case B_LARGE_ICON:
				type = B_LARGE_ICON_TYPE;
				break;
			default:
				return NULL;
		}
#endif

		// Fetch bitmap icon
		data = resources->LoadResource(type, num, &nbytes);
		cspace = B_CMAP8;
		if (data == NULL)
			return NULL;
	}

	BBitmap* icon = new BBitmap(BRect(0, 0, size - 1, size - 1), cspace);
	if (icon->InitCheck() < B_OK)
		return NULL;

	switch (type) {
#ifdef __HAIKU__
		case B_VECTOR_ICON_TYPE:
			if (BIconUtils::GetVectorIcon((const uint8*)data, nbytes, icon) < B_OK) {
				delete icon;
				return NULL;
			}
			break;
#endif
		default:
			icon->SetBits(data, size * size, 0, cspace);
	}

	return icon;
}

// Reads attribute from node. Returns contents (to be free()'d by user) or NULL on
// fail

char *ReadAttribute(BNode node, const char *attribute, int32 *length) {
	attr_info info;
	char *value = NULL;

	if (node.GetAttrInfo(attribute, &info) == B_OK) {
		value = (char *)calloc(info.size, sizeof(char));
		if (node.ReadAttr(attribute, info.type, 0, (void *)value, info.size) !=
			info.size) {
			
			free(value);
			value = NULL;
		};
		if (length) *length = info.size;
	};

	return value;
};

status_t WriteAttribute(BNode node, const char *attribute, const char *value,
	size_t length, type_code type) {

	status_t ret = B_ERROR;
	if ((ret = node.InitCheck()) == B_OK) {
		ret = node.WriteAttr(attribute, type, 0, value, length);
	};
	
	return ret;
};


BBitmap * rescale_bitmap( const BBitmap * src, int32 width, int32 height )
{
	width--; height--;
	
	if (!src || !src->IsValid()) return NULL;
	
	BRect srcSize = src->Bounds();
	
	if ( height < 0 )
	{
		float srcProp = srcSize.Height() / srcSize.Width();
		height = (int32)(width * srcProp);
	}
	
	BBitmap * res = new BBitmap( BRect(0,0,width,height), src->ColorSpace() );
	
	float dx = (srcSize.Width()+1) / (width+1);
	float dy = (srcSize.Height()+1) / (height+1);
		
	uint8 bpp = (uint8)(src->BytesPerRow() / srcSize.Width());
	
	int srcYOff = src->BytesPerRow();
	int dstYOff = res->BytesPerRow();
	
	void * dstData = res->Bits();
	void * srcData = src->Bits();
	
	for ( int y=0; y<=height; y++ )
	{
		void *dstRow = (void *)((uint32)dstData + (uint32)(y*dstYOff));
		void * srcRow = (void *)((uint32)srcData + ((uint32)(y*dy)*srcYOff));
		
		for ( int x=0; x<=width; x++ )
		{
			memcpy((void *)((uint32)dstRow+(x*bpp)),(void *)((uint32)srcRow+((uint32)(x*dx)*bpp)),bpp);
		}
	}
	
	return res;
}

extern "C" {

status_t
our_image(image_info& image)
{
	team_id team = 0;
#ifdef __HAIKU__
	team = B_CURRENT_TEAM;
#endif

	int32 cookie = 0;
	while (get_next_image_info(team, &cookie, &image) == B_OK) {
		if ((char *)our_image >= (char *)image.text
			&& (char *)our_image <= (char *)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}

}
