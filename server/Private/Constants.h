#ifndef PRIVATE_CONSTANTS_H
#define PRIVATE_CONSTANTS_H

namespace IM {
	namespace Private {

		enum what_code	{
			PROTOCOL_STARTED = 'PSTR',
			PROTOCOL_STOPPED = 'PSTP',
			PROTOCOL_PROCESS = 'PPRO',
			PROTOCOL_UPDATESETTINGS = 'PUPS',
			PROTOCOL_KILLED = 'PCSH',
		};
	};
};

#endif
