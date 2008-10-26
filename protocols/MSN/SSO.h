#ifndef MSN_SSO_H
#define MSN_SSO_H

#include <String.h>

#include <map>

class HTTPFormatter;
class SSO;
class SSOHandler;

typedef status_t (SSO::*SSOMethod)(BString, BString, BString &);
typedef map<BString, SSOMethod> ssomethod_t;

class SSO {
	public:
							SSO(SSOHandler *handler, const char *passport, const char *password,
								const char *URI, const char *nonce);
							~SSO(void);
			
		status_t			Response(BString &token, BString &response);
							
	private:
		status_t			SSLSend(const char *host, HTTPFormatter *const send,
								HTTPFormatter **recv);
	
		status_t			MBIKeyOld(BString key, BString nonce, BString &response);

		SSOHandler			*fHandler;
		BString				fPassport;
		BString				fPassword;
		BString				fURI;
		BString				fNonce;

		ssomethod_t			fMethods;
};

#endif
