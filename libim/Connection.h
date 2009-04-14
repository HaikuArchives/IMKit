#ifndef IM_CONNECTION_H
#define IM_CONNECTION_H

#include <String.h>

namespace IM {

	class Connection {
		public:
								Connection(void);
								Connection(const char *);
								Connection(const char *protocol, const char *account, const char *ID);
								Connection(const Connection &);
	
			// Public
			const char 			*Protocol(void) const;
			const char			*ID(void) const;
			const char			*Account(void) const;
			bool				HasAccount(void) const;

			const char 			*String(void) const;
	
			// Operators
			bool 				operator == (const Connection &) const;
			
		private:
			BString				fConn;
			BString				fProtocol;
			BString				fID;
			BString				fAccount;
	};
};

#endif // IM_CONNECTION_H
