/*
 * Copyright 2003-2009, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */
 
#ifndef CONTACTCACHEDCONNECTIONS_H
#define CONTACTCACHEDCONNECTIONS_H
 
#include <storage/Entry.h>
 
#include <libim/Contact.h>
 
namespace IM {
	class ConnectionStore;

	class ContactCachedConnections : public Contact {
		public:
							ContactCachedConnections(entry_ref ref);
		
			// Public
			void			ReloadConnections(void);
			ConnectionStore *CachedConnections(void);

			// Operators
							operator const entry_ref *(void) const;
			bool			operator == (const entry_ref & entry) const;
	
		private:
			ConnectionStore	*fConnections;
	};
	
};

#endif // CONTACTCACHEDCONNECTIONS_H
