#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <String.h>
#include <Message.h>
#include <string>

class Tracer
{
	private:
		BString str;
		
	public:
		Tracer( const char * _str ) 
		: str(_str) 
		{ 
			printf("%s\n",str.String()); 
		};
		~Tracer() 
		{ 
			printf("-%s\n",str.String() );
		};
};

enum log_importance {
	liDebug = 0,	// debug
	liLow,			// lots and lots of messages
	liMedium,		// enough to follow what's happening
	liHigh,			// general idea
	liQuiet = 100	// Don't say a word. Don't use this when calling LOG(), it's for reducing output.
};

extern void LOG( const char * module, log_importance level, const char * msg, const BMessage *, ...);
extern void LOG( const char * module, log_importance level, const char * msg, ...);

// if level < g_verbosity_level then LOG() doesn't print the msg
extern log_importance g_verbosity_level;

// settings management.
extern status_t im_load_protocol_settings( const char * protocol, BMessage * settings );
extern status_t im_load_protocol_template(const char *protocol, BMessage* msg);
extern status_t im_load_protocol_accounts(const char* protocol, BMessage* msg);
extern status_t im_load_client_settings( const char * client, BMessage * settings );
extern status_t im_load_client_template( const char * client, BMessage * tmplate );

extern status_t im_save_protocol_settings( const char * protocol, const BMessage * settings );
extern status_t im_save_protocol_template( const char * protocol, const BMessage * tmplate );
extern status_t im_save_client_settings( const char * client, const BMessage * settings );
extern status_t im_save_client_template( const char * client, const BMessage * tmplate );

/*
	Returns a message containing string(s) named "protocol" with the name of the protocols
*/
extern void im_get_protocol_list( BMessage * list );
/*
	Returns a message containing string(s) named "client" with the name of the protocols
*/
extern void im_get_client_list( BMessage * list );

extern void crlf2nl( const char * orig, BString & conv );
extern void nl2crlf( const char * orig, BString & conv );

extern string connection_protocol( string connection );
extern string connection_id( string connection );

#endif
