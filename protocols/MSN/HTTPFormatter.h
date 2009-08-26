#ifndef HTTPFORMATTER_H
#define HTTPFORMATTER_H

#include <String.h>

#include <map>
#include <vector>

typedef std::map<BString, BString> HeaderMap;

class HTTPFormatter {
	public:
					HTTPFormatter(void);
					HTTPFormatter(const char *host, const char *document);
					HTTPFormatter(const char *response, int32 length);
					~HTTPFormatter();

		status_t	Host(const char	*host);
		const char	*Host(void);
		
		status_t	Document(const char *document);
		const char	*Document(void);
					
		status_t	Version(const char *version);
		const char 	*Version(void);
		
		status_t	RequestType(const char *request);
		const char	*RequestType(void);
		
		status_t	AddHeader(const char *name, const char *value);
		status_t	ClearHeaders(void);

		const char	*HeaderContents(const char *name);
		status_t	HeaderContents(const char *header, BString &value);

		int32		Headers(void) { return fHeaders.size(); };
		const char	*HeaderNameAt(int32 index);
		const char	*HeaderAt(int32 index);

		status_t	SetContent(const char *content, size_t length);
		status_t	AppendContent(const char *content, size_t length);
		const char	*Content(void);
		int32		ContentLength(void);
		status_t	ClearContent(void);
		
		status_t	Clear(void);

		int32		Length(void);
		const char	*Flatten(void);
	
		int16		Status(void) { return fStatus; };
	
	private:
		void		_init(void);
	
		BString		fHost;
		BString		fDocument;
		BString		fVersion;
		BString		fRequestType;
		
		HeaderMap	fHeaders;
					
		BString		fContent;
		int16		fStatus;

		bool		fDirty;
		BString		fFlattened;		
};

#endif
