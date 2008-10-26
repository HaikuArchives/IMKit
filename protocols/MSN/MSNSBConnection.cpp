#include "MSNSBConnection.h"

#include "MSNConnection.h"
#include "Buddy.h"

#include <algorithm>

//#pragma mark Constructor

MSNSBConnection::MSNSBConnection(const char *server, uint16 port, MSNManager *man)
	: MSNConnection(server, port, man) {
};

MSNSBConnection::~MSNSBConnection(void) {
};

//#pragma mark BLooper Hooks

void MSNSBConnection::MessageReceived(BMessage * msg) {
	switch (msg->what) {
		case msnmsgPing: {
			// Ignore pings
		} break;
		
		default: {
			MSNConnection::MessageReceived(msg);
		} break;
	};
};

//#pragma mark Public

bool MSNSBConnection::IsGroupChat(void) const {
	return fParticipants.size() > 1;
};

bool MSNSBConnection::IsSingleChatWith(const char * who) {
	participant_t::iterator i = fParticipants.begin();
	
	return ((fParticipants.size() == 1) && (strcasecmp((*i)->Passport(), who) == 0));
};

bool MSNSBConnection::InChat(const char * who) {
	participant_t::iterator i;
	
	for ( i = fParticipants.begin(); i != fParticipants.end(); i++ ) {
		if (strcmp((*i)->Passport(), who) == 0) break;
	};
	
	return (i == fParticipants.end());
};

status_t MSNSBConnection::HandleCAL(Command * cmd) {
	// Invite someone (response)
	LOG(kProtocolName, liDebug, "C %lX: Processing CAL (SB)", this);
	return B_OK;
};

void MSNSBConnection::SendMessage(Command * cmd) {
	// Queue the message if no one is in the chat otherwise send it
	if (fParticipants.empty() == true ) {
		fPendingMessages.push_back(cmd);
	} else {
		Send(cmd);
	};
};

//#pragma mark Protected

status_t MSNSBConnection::HandleJOI(Command * cmd) {
	// someone new in chat
	LOG(kProtocolName, liDebug, "C %lX: Processing JOI (SB): %s", this, cmd->Param(0));
	
	fParticipants.push_back(fManager->BuddyDetails(cmd->Param(0)));
	
	// send any pending messages
	pendingmsg_t::iterator pIt;
	for (pIt = fPendingMessages.begin(); pIt != fPendingMessages.end(); pIt++) {
		Send(*pIt);
	};
	fPendingMessages.clear();
	
	return B_OK;
};

status_t MSNSBConnection::HandleIRO(Command * cmd ) {
	// List of those already in chat
	LOG(kProtocolName, liDebug, "C %lX: Processing IRO (SB): %s", this, cmd->Param(2));

	Buddy *bud = fManager->BuddyDetails(cmd->Param(2));
	fParticipants.push_back(bud);

//	if (bud) {
//		MSNObject *obj = bud->DisplayPicture();
//		if (obj) {
//			Command *com = new Command("MSG");
//			com->AddParam("A");
//			com->AddPayload("MIME-Version: 1.0\r\n");
//			com->AddPayload("Content-Type: application/x-msnmsgrp2p\r\n");
//			com->AddPayload("P2P-Dest: imkitaim@netscape.net\r\n\r\n");
//	
//			P2PHeader *head = new P2PHeader("INVITE", bud->Passport());
//			head->AddField("To","<msnmsgr:imkitaim@netscape.net>");
//			head->AddField("From", "<msnmsgr:industroslaad@netscape.net>");
//			head->AddField("Via", "MSNLP/1.0/TLP ;branch={33517CE4-02FC-4428-B6F4-39927229B722}");
//			head->AddField("CSeq", "0 ");
//			head->AddField("Call-ID", "{9D79AE57-1BD5-444B-B14E-3FC9BB2B5D58}");
//			head->AddField("Max-Forwards", "0");
//			head->AddField("Content-Type", "application/x-msnmsgr-sessionreqbody");
//			head->Identifier(132413);
//			head->AckSessionID(24321451);
//			head->AckUniqueID(3214213);
//			head->Flags(0);
//	
//			P2PContents *content = new P2PContents();
//			content->AddField("EUF-GUID", "{A4268EEC-FEC5-49E5-95C3-F126696BDBF6}");
//			content->AddField("SessionID", "1980589");
//			content->AddField("AppID", "1");
//	
//			content->AddField("Context", obj->Base64Encoded());
//	
//			head->Content(content);
//			printf("Head:\n");
//			head->Debug();
////			
//			com->AddPayload(head->Flatten(), head->FlattenedLength());
//			printf("--\n\n\n\n\n--Command:\n");
//			com->Debug();
//			
//			Send(com);
//		};
//	};
	
	return B_OK;
};

/**
	Fully connected (got list of participants), send any pending messages
*/
status_t MSNSBConnection::HandleANS(Command * cmd) {
	pendingmsg_t::iterator pIt;
	for (pIt = fPendingMessages.begin(); pIt != fPendingMessages.end(); pIt++) {
		Send(*pIt);
	};
	
	fPendingMessages.clear();
	
	return B_OK;
};

status_t MSNSBConnection::HandleBYE(Command * cmd) {
	// Someone left the conversation
	LOG(kProtocolName, liDebug, "C %lX: Processing BYE (SB): %s left.", this, cmd->Param(0));
	
	fParticipants.remove(fManager->BuddyDetails(cmd->Param(0)));
	
	// No one else in the chat, leave
	if (fParticipants.empty() == true) {
		Command *reply = new Command("OUT");
		reply->UseTrID(false);
		
		Send(reply, qsImmediate);
		
		BMessage closeCon(msnmsgCloseConnection);
		closeCon.AddPointer("connection", this);

		fManMsgr.SendMessage(&closeCon);
	};
	
	return B_OK;
};

status_t MSNSBConnection::HandleUSR(Command * cmd) {
	// Just send any pending messages here.
	LOG(kProtocolName, liDebug, "C %lX: Processing USR (SB)", this);
	
	GoOnline();
	
	return B_OK;
};
