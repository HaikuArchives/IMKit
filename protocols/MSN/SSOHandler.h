#ifndef MSN_SSOHANDLER_H
#define MSN_SSOHANDLER_H

class SSOHandler {
	public:
		virtual inline		~SSOHandler(void) { };
		
		virtual void		SSORequestingTicket(void) = 0;
		virtual void		SSOGeneratingResponse(void) = 0;
		virtual void		SSOError(const char *error) = 0;
};

#endif
