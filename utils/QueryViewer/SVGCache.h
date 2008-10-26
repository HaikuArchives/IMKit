#ifndef SVG_CACHE_H
#define SVG_CACHE_H

#include <OS.h>

#ifdef B_ZETA_VERSION

class BBitmap;

extern BBitmap * getBitmap( const char * path, int32 size, bool use_cache = true );

class CacheTimer
{
	public:
		bigtime_t start;
		bool hit;
		bool active;
		
		static bigtime_t totalHitTime;
		static bigtime_t totalMissTime;
		static int32 numHits;
		static int32 numMisses;
		
		CacheTimer()
		:	start( system_time() ),
			hit(true),
			active(true)
		{
		}
		
		~CacheTimer()
		{
			if ( !active )
				return;
			
			bigtime_t t = system_time() - start;
			if ( hit )
			{
				numHits++;
				totalHitTime += t;
			} else 
			{
				numMisses++;
				totalMissTime += t;
			}
		}	
		
		void Miss()
		{
			hit = false;
		}
		
		void Cancel()
		{
			active = false;
		}
};

#else
#define getBitmap ReadNodeIcon
#endif

#endif
