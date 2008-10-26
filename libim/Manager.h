#ifndef IM_MANAGER_H
#define IM_MANAGER_H

#include <Messenger.h>
#include <Looper.h>

namespace IM {

class Manager : public BLooper
{
	public:
		Manager();
		Manager( BMessenger target );
		~Manager();
		
		void StartListening();
		void StopListening();
		
		static status_t OneShotMessage( BMessage * msg );
		
		status_t SendMessage( BMessage * msg, BMessage * reply=NULL );
		
		void FlashDeskbar( BMessenger );
		void StopFlashingDeskbar( BMessenger );
		
		// 
		void MessageReceived( BMessage * );
		
		status_t	InitCheck();
		
	private:
		void AddEndpoint( BMessenger );
		void RemoveEndpoint( BMessenger );
		
		static const bigtime_t fTimeout=100*1000;
		
		bool fIsListening;
		bool fIsRegistered;
		BMessenger	fMsgr, fTarget;
};

};

#endif
