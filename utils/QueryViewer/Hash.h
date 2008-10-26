#ifndef HASH_H
#define HASH_H

#include <OS.h>

class Hash
{
	protected:
		Hash( int32 );
		Hash( const Hash & );
		
		char * fHash;
		int32 fHashSize;

	public:
		virtual ~Hash();
		
		bool operator == ( const Hash & ) const;
		bool operator < ( const Hash & ) const;
		
		virtual void CalcHash( const char *, int32 )=0;
};

class MD4Hash : public Hash
{
	public:
		MD4Hash();
		MD4Hash( const MD4Hash & );
		void CalcHash( const char *, int32 );
};



#endif
