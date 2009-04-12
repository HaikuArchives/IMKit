#include "ProtocolHandler.h"

#include <libim/Constants.h>

//#pragma mark Constructor

ProtocolHandler::ProtocolHandler(const char *instance)
	: BHandler(instance),
	fInstance(instance),
	fIMServer(IM_SERVER_SIG) {
};

//#pragma mark BHandler Hooks

void ProtocolHandler::MessageReceived(BMessage *msg) {
	msg->AddString("instance_id", fInstance);
	fIMServer.SendMessage(msg);
};