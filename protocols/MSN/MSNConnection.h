#ifndef MSNCONNECTION_H
#define MSNCONNECTION_H

#include <UTF8.h>
#include <Handler.h>
#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>

#include <list>
#include <map>
#include <vector>

#include <libim/Helpers.h>
#include <libim/Protocol.h>
#include <libim/Constants.h>

#include "MSNManager.h"
#include "MSNHandler.h"
#include "Command.h"
#include "HTTPFormatter.h"
#include "P2PHeader.h"
#include "P2PContents.h"

// Member Function Pointer for command handlers
typedef status_t (MSNConnection::*CommandHandler)(Command *);
typedef std::map<BString, CommandHandler> commandhandler_t;
typedef std::pair <char *, int16> ServerAddress;
typedef std::list<Command *> CommandQueue;
typedef std::list<BString> contact_t;

class MSNManager;

class MSNConnection : public BLooper {
	public:
							MSNConnection(void);
							MSNConnection(const char *server, uint16 port, MSNManager *man);
							~MSNConnection(void);
						
		// BLooper hooks
		void				MessageReceived(BMessage *msg);
		bool				QuitRequested(void);

		// Accessors
		const char 			*Server(void) const;
		uint16				Port(void) const;
		bool				IsConnected(void);

		// Public Methods
		void				SetTo(const char *server, uint16 port, MSNManager *man);
		status_t			Send(Command *command, queuestyle queue = qsQueue);
		status_t			ProcessCommand(Command *command);

		
	protected:
		virtual status_t	HandleVER(Command *command);
		virtual status_t	HandleNLN(Command *command);
		virtual status_t	HandleCVR(Command *command);
		virtual status_t	HandleRNG(Command *command);
		virtual status_t 	HandleXFR(Command *command);
		virtual status_t	HandleCHL(Command *command);
		virtual status_t	HandleUSR(Command *command);
		virtual status_t	HandleMSG(Command *command);
		virtual status_t	HandleADC(Command *command);
		virtual status_t	HandleLST(Command *command);
		virtual status_t 	HandleQRY(Command *command);
		virtual status_t 	HandleGTC(Command *command);
		virtual status_t 	HandleBLP(Command *command);
		virtual status_t 	HandlePRP(Command *command);
		virtual status_t 	HandleCHG(Command *command);
		virtual status_t 	HandleFLN(Command *command);
		virtual status_t 	HandleSYN(Command *command);
		virtual status_t 	HandleJOI(Command *command);
		virtual status_t 	HandleCAL(Command *command);
		virtual status_t 	HandleIRO(Command *command);
		virtual status_t 	HandleANS(Command *command);
		virtual status_t 	HandleBYE(Command *command);
		virtual status_t	HandleSBS(Command *command);
		virtual status_t	HandleILN(Command *command);
		
		void				StartReceiver(void);
		void				StopReceiver(void);
		void				GoOnline(void);
		void				ClearQueues(void);
		
		ServerAddress		ExtractServerDetails(char *details);
		
		MSNManager			*fManager;	
		BMessenger			fManMsgr;

	private:
		void				SetupCommandHandlers(void);
		int32				NetworkSend(Command *command);
		int32				ConnectTo(const char *hostname, uint16 port);
		static int32		Receiver(void *con);
		status_t			SSLSend(const char *host, HTTPFormatter *send, HTTPFormatter **recv);
		void				Error(const char *message, bool disconnected = false);
		void				Progress(const char * id, const char * msg, float progress);

		commandhandler_t	fCommandHandler;
		
		contact_t			fContacts;
		
		char				*fServer;
		uint16				fPort;
		
		CommandQueue		fOutgoing;
		CommandQueue		fWaitingOnline;
		uint32				fTrID;
		
		BMessenger			*fSockMsgr;
		BMessageRunner		*fRunner;
		BMessageRunner		*fKeepAliveRunner;
		
		int16				fSock;

		uint8				fState;
		thread_id			fThread;
};

#endif
