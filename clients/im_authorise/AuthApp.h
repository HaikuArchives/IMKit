#ifndef AUTHAPP_H
#define AUTHAPP_H

// Be API 
#include <Application.h>
#include <Entry.h>

// IM API
#include <libim/Manager.h>

// STL
#include <map>

// Forward declarations
class AuthWindow;

// Typedefs
typedef map<entry_ref, AuthWindow *> window_t;

// Constants
extern const int32 kAuthorise;
extern const int32 kDeny;

class AuthApp : public BApplication {
	public:
						AuthApp(void);
						~AuthApp(void);
				
				
		// BApplication Hooks
		void 			MessageReceived(BMessage *msg);

	private:
		void			UpdateSettings(void);
	
		IM::Manager		*fManager;
};

#endif
