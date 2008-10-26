#include "Manager.h"

#include "Constants.h"
#include "Helpers.h"

#include <stdio.h>
#include <Roster.h>

using namespace IM;

Manager::Manager()
:	fIsListening(false),
	fIsRegistered(false),
	fMsgr(IM_SERVER_SIG)
{
}

Manager::Manager( BMessenger target )
:	fIsListening(false),
	fIsRegistered(false),
	fMsgr(IM_SERVER_SIG),
	fTarget(target)
{
	Run();
	
	be_roster->StartWatching( BMessenger(this) );
}

Manager::~Manager()
{
	be_roster->StopWatching( BMessenger(this) );
	
	StopListening();
	
//	Lock(); // ????
}

void
Manager::MessageReceived( BMessage * msg )
{
	switch ( msg->what )
	{
		case B_SOME_APP_LAUNCHED:
		{
			//printf("B_SOME_APP_LAUNCHED\n");
			if ( strcmp(msg->FindString("be:signature"), IM_SERVER_SIG ) == 0)
			{ // im_server started, update messenger
				
				// this is due to a bug. constructing a messenger right away
				// results in an invalid messenger, if we wait a little while
				// it will be ok. Seems like the Roster is a bit too eager to
				// announce the start of an app..
				snooze(1000*1000);
				
				fMsgr = BMessenger(IM_SERVER_SIG);
				
				if ( fIsListening && !fIsRegistered )
				{ // we're listening, add endpoint
					AddEndpoint( fTarget );
				}
			}
		}	break;
		
		case B_SOME_APP_QUIT:
		{
			//printf("B_SOME_APP_QUIT\n");
			if ( 
				fIsRegistered && 
				(strcmp(msg->FindString("be:signature"), IM_SERVER_SIG ) == 0)
			)
			{ // im_server quit, flip isRegistered switch
//				printf("Removing endpoint\n");
				fIsRegistered = false;
			}
		}	break;
		
		default:
			BLooper::MessageReceived(msg);
	}
}

void
Manager::StartListening()
{
	if ( !fIsListening )
	{
		fIsListening = true;
		AddEndpoint( fTarget );
	}
}

void
Manager::StopListening()
{
	RemoveEndpoint( fTarget );
	
	fIsListening = false;
}

/**
	Adds a BMessenger that will receive events from the im_server
*/
void
Manager::AddEndpoint( BMessenger msgr )
{
	BMessage msg(ADD_ENDPOINT), reply;
	msg.AddMessenger("messenger",msgr);

	status_t result = fMsgr.SendMessage(&msg,&reply);
	
	if ( reply.what == IM::ACTION_PERFORMED )
	{
		fIsRegistered = true;
	} 
	
	switch ( result )
	{
		case B_OK:
			break;
		case B_TIMED_OUT:
			LOG("Manager", liHigh, "Manager::AddEndpoint: B_TIMED_OUT");
			break;
		case B_WOULD_BLOCK:
			LOG("Manager", liHigh, "Manager::AddEndpoint: B_WOULD_BLOCK");
			break;
		case B_BAD_PORT_ID:
			LOG("Manager", liHigh, "Manager::AddEndpoint: B_BAD_PORT_ID");
			break;
		case B_NO_MORE_PORTS:
			LOG("Manager", liHigh, "Manager::AddEndpoint: B_NO_MORE_PORTS");
			break;
		default:
			LOG("Manager", liHigh, "Manager::AddEndpoint: Other error");
	}
}

/**
	Removes a BMessenger from the im_server's subscriber-list
*/
void
Manager::RemoveEndpoint( BMessenger msgr )
{
	BMessage msg(REMOVE_ENDPOINT);
	msg.AddMessenger("messenger",msgr);

	fMsgr.SendMessage(&msg);
	
	fIsRegistered = false;
}

/**
	Forward a single, one-shot, message to the im_server
*/
status_t
Manager::OneShotMessage( BMessage * msg )
{
	BMessenger msgr(IM_SERVER_SIG);
	
	if ( !msgr.IsValid() )
	{
		LOG("Manager", liHigh, "Manager::SendMessage: fMsgr invalid");
		return B_ERROR;
	}
	
	return msgr.SendMessage(msg,(BHandler*)NULL);
}

/**
	Forward a message to the im_server
*/
status_t
Manager::SendMessage( BMessage * msg, BMessage * reply = NULL )
{
	if ( !fMsgr.IsValid() )
	{
		LOG("Manager", liHigh, "Manager::SendMessage: fMsgr invalid");
		return B_ERROR;
	}
	
	status_t result;
	
	if ( reply )
	{
		result = fMsgr.SendMessage(msg,reply,fTimeout,fTimeout);
	} else
	{
		result = fMsgr.SendMessage(msg,(BHandler*)NULL,fTimeout);
	}
	
	return result;
}

void
Manager::FlashDeskbar( BMessenger msgr )
{
	BMessage msg(FLASH_DESKBAR), reply;
	msg.AddMessenger("messenger",msgr);
	
	SendMessage(&msg);
}

void
Manager::StopFlashingDeskbar( BMessenger msgr )
{
	BMessage msg(STOP_FLASHING), reply;
	msg.AddMessenger("messenger",msgr);
	
	SendMessage(&msg);
}

/**
	Returns B_OK if im_server is running.
*/
status_t
Manager::InitCheck()
{
	if ( fMsgr.IsValid() )
		return B_OK;
	
	return B_ERROR;
}
