#include "SVGCache.h"

#ifdef B_ZETA_VERSION

#include <sys_apps/Tracker/Icons.h>
#include <Bitmap.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <Entry.h>
#include <Directory.h>
#include <stdio.h>
#include <Path.h>
#include <Application.h>

#include "Hash.h"
#include "IMKitUtilities.h"

//-------------------------------------------------------------

#define BEOS_SVG_ICON_ATTRIBUTE		"BEOS:V:STD_ICON"

//-------------------------------------------------------------

bigtime_t CacheTimer::totalHitTime = 0;
bigtime_t CacheTimer::totalMissTime = 0;
int32 CacheTimer::numHits = 0;
int32 CacheTimer::numMisses = 0;

class BitmapCacheKey
{
	public:
		BitmapCacheKey( MD4Hash, int32 );
		BitmapCacheKey( const BitmapCacheKey & );
		MD4Hash	fHash;
		int32	fSize;
		
		bool operator < ( const BitmapCacheKey & ) const;
		bool operator == ( const BitmapCacheKey & ) const;
};

BitmapCacheKey::BitmapCacheKey( MD4Hash h, int32 s )
:	fHash( h ),
	fSize( s )
{
}

BitmapCacheKey::BitmapCacheKey( const BitmapCacheKey & k )
:	fHash( k.fHash ),
	fSize( k.fSize )
{
}

bool
BitmapCacheKey::operator == ( const BitmapCacheKey & k ) const
{
	return fSize == k.fSize && fHash == k.fHash;
}

bool
BitmapCacheKey::operator < ( const BitmapCacheKey & k ) const
{
	return fSize < k.fSize || fHash < k.fHash;
}

//-------------------------------------------------------------

typedef map<BitmapCacheKey, BBitmap *>  BitmapCache;
BitmapCache gBitmapCache;

//-------------------------------------------------------------

/*
	First tries to read SVG data from the node.
	If this fails, it returns NULL.
	Otherwise it calculates the MD4 sum of the SVG data, then
	checks the cache for a matching value and bitmap size.
	If found, it returns a copy of the bitmap.
	If not, a bitmap is rendered and added to the cache before
	a copy of the bitmap is returned.
*/
BBitmap *
getBitmap( const char * path, int32 size, bool use_cache = true )
{
	CacheTimer timer;
	
	// get svg data
	int32 dataSize=0;
	char * iconData = ReadAttribute( path, BEOS_SVG_ICON_ATTRIBUTE, &dataSize );
	
	if (dataSize == 0) {
		// No local SVG data found, return Tracker icon and cancel timer	
		timer.Cancel();
		return new BBitmap(Z::Tracker::GetTrackerIcon(BEntry(path), size));
	}
	
	// calc hash
	MD4Hash hash;
	
	hash.CalcHash( iconData, dataSize );
	
	// create key
	BitmapCacheKey key(hash,size);
	
	// look in cache
	if ( use_cache )
	{
		BitmapCache::const_iterator iter;
		iter = gBitmapCache.find( key );
	
		if ( iter != gBitmapCache.end() )
		{
			// found it
			return new BBitmap( iter->second );
		}
	}
	
	// didn't find it, render and cache
	timer.Miss();
	
	BBitmap * bitmap = new BBitmap(Z::Tracker::GetTrackerIcon(BEntry(path), size));
	
	gBitmapCache[key] = bitmap;
	
	return new BBitmap( bitmap );
}

//-------------------------------------------------------------

/*
int main222( int numArg, const char ** argv )
{
	BApplication app("application/x-vnd.m_eiman.test_svg_cache");
	
	bool use_cache = true;
	
	// some argument management
	if ( numArg < 2 )
	{
		printf("Usage: icon_cache_test dir_to_work_in [bypass_cache]\n");
		return 1;
	}
	
	if ( numArg == 3 && strcmp(argv[2], "bypass_cache") == 0 )
		use_cache = false;
	
	// Start doing the actual work here
	BDirectory dir(argv[1]);
	
	BEntry entry;
	BPath path;
	while ( dir.GetNextEntry(&entry, true) == B_OK )
	{
		entry.GetPath( &path );
		
		BBitmap * bmp = getBitmap( path.Path(), 32, use_cache );
		
		if ( bmp)
			delete bmp;
	}
	
	// Print some stats
	printf("Total hits: %ld, mean time per hit: %.0f\n",
		CacheTimer::numHits, CacheTimer::totalHitTime / (float)CacheTimer::numHits
	);
	
	printf("Total misses: %ld, mean time per miss: %.0f\n",
		CacheTimer::numMisses, CacheTimer::totalMissTime / (float)CacheTimer::numMisses
	);
	
	return 0;
}
*/

#endif
