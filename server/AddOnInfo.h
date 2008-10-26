#ifndef ADD_ON_INFO_H
#define ADD_ON_INFO_H

#include <libim/Protocol.h>
#include <support/String.h>
#include <OS.h>

namespace IM {

class AddOnInfo
{
	public:
		Protocol		* protocol;
		const char		* signature;
		BString			path;
		image_id		image;
};

};

#endif
