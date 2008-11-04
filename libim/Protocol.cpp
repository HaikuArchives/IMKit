#include "Protocol.h"

using namespace IM;

Protocol::Protocol( uint32 capabilities ) 
:	m_capabilities( capabilities )
{
}

Protocol::~Protocol() {
}

bool
Protocol::HasCapability( capability_bitmask cap )
{
	return (m_capabilities & cap) != 0;
}

uint32
Protocol::Capabilities(void) {
	return m_capabilities;
};