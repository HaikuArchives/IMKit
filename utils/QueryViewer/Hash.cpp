#include "Hash.h"

#include <string.h>
#include <stdlib.h>

#include "md4checksum.h"

//-------------------------------------------------------------

Hash::Hash( int32 size )
:	fHashSize(size)
{
	fHash = new char[fHashSize];
	memset( fHash, 0, fHashSize );
}

Hash::Hash( const Hash & h )
:	fHashSize( h.fHashSize )
{
	fHash = new char[fHashSize];
	memcpy( fHash, h.fHash, fHashSize );
}

Hash::~Hash()
{
	delete fHash;
}

bool
Hash::operator == ( const Hash & h ) const
{
	return fHashSize == h.fHashSize && memcmp(fHash, h.fHash, fHashSize) == 0;
}

bool
Hash::operator < ( const Hash & h ) const
{
	if ( fHashSize < h.fHashSize )
		return true;
	if ( fHashSize > h.fHashSize )
		return false;
	
	return memcmp(fHash, h.fHash, fHashSize) < 0;
}

//-------------------------------------------------------------

MD4Hash::MD4Hash()
:	Hash(16)
{
}

MD4Hash::MD4Hash( const MD4Hash & sh )
:	Hash(sh)
{
}

void
MD4Hash::CalcHash( const char * data, int32 size )
{
	MD4Checksum md4;
	md4.Process( (char*)data, size );
	md4.GetResult( fHash );
}
