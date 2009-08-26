#ifndef MSN_SB_CONNECTION_H
#define MSN_SB_CONNECTION_H

#include "MSNConnection.h"

#include <string>
#include <list>

class Buddy;

typedef std::list<Buddy *> participant_t;
typedef std::list<Command *> pendingmsg_t;

class MSNSBConnection : public MSNConnection {
	public:
							MSNSBConnection(const char *server, uint16 port, MSNManager *man);
							~MSNSBConnection(void);
		
		// BLooper Hooks
		virtual void		MessageReceived(BMessage *msg);
		
		// Public
		bool 				IsGroupChat(void) const;
		bool				IsSingleChatWith(const char *who);
		bool				InChat(const char *who);
		
		/**
			Send a message to someone. If no participants have joined
			yet, store the message until they have and then send.
		*/
		void 				SendMessage(Command *cmd);
		
	protected:
		participant_t		fParticipants;
		pendingmsg_t		fPendingMessages;
		
		virtual status_t	HandleCAL(Command *cmd);
		virtual status_t 	HandleJOI(Command *cmd);
		virtual status_t 	HandleIRO(Command *cmd);
		virtual status_t 	HandleBYE(Command *cmd);
		virtual status_t 	HandleUSR(Command *cmd);
		virtual status_t 	HandleANS(Command *cmd);

};

#endif
