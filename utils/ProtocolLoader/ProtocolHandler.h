/*
 * Copyright 2009-, IM Kit Team.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson <slaad@bong.com.au>
 *		
 */

#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <app/Handler.h>
#include <app/Messenger.h>
#include <support/String.h>

class ProtocolHandler : public BHandler {
	public:
							ProtocolHandler(const char *instance);
	
		// BHandler Hooks
		void				MessageReceived(BMessage *msg);
	
	private:
		BString				fInstance;
		BMessenger			fIMServer;
};

#endif
