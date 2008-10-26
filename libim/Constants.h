#ifndef IM_CONSTANTS_H
#define IM_CONSTANTS_H

namespace IM {

/**
	Defines different IM messages
*/
enum im_what_code {
	/* Request a server-side contact list from protocol */
	GET_CONTACT_LIST	= 1,
	// Send a message to a contact
	SEND_MESSAGE		= 2,
	// Message has been sent to contact
	MESSAGE_SENT		= 3,
	// Message received from contact
	MESSAGE_RECEIVED	= 4,
	// Contact's status has changes
	STATUS_CHANGED		= 5,
	// Server-side contact list received
	CONTACT_LIST		= 6,
	// Change own status
	SET_STATUS			= 7,
	// Retreive information on contact
	GET_CONTACT_INFO	= 8,
	// Received information on contact
	CONTACT_INFO		= 9,
	// Start listening to changes in these contact's statuses
	REGISTER_CONTACTS	= 10, // provide a list of contacts we're interested in to the protocol
	// These are rather obvious, no?
	CONTACT_STARTED_TYPING	= 11,
	CONTACT_STOPPED_TYPING	= 12,
	USER_STARTED_TYPING		= 13,
	USER_STOPPED_TYPING		= 14,
	// Own status was chagned
	STATUS_SET			= 15,
	// Authorization request received
	AUTH_REQUEST		= 16,
	// Send authorization
	SEND_AUTH_ACK		= 17,
	// Contact has been authorized
	CONTACT_AUTHORIZED	= 18,
	// Request authorization from contact
	REQUEST_AUTH		= 19,
	// Stop listening to status changes from these contacts
	UNREGISTER_CONTACTS = 20,
	// Progress message received, could be login sequence, file transfer etc
	PROGRESS			= 21,
	
	GET_AWAY_MESSAGE =  22,
	AWAY_MESSAGE = 23,
	
	// Protocols send this when they get a new buddy icon
	SET_BUDDY_ICON = 24,
	// Clients get this when a buddy icon is updated
	BUDDY_ICON_UPDATED = 25,
	
	// Adding and removing contact from the server side list
	SERVER_LIST_ADD_CONTACT = 26,
	SERVER_LIST_REMOVED_CONTACT = 27,
	
	// these are forwarded to 
	SPECIAL_TO_PROTOCOL = 1000,
	SPECIAL_FROM_PROTOCOL = 1001
};

/**
	what-codes for messages sent to and from the im_server
*/
enum message_what_codes {
	/*
		Used for all error messages
	*/
	ERROR				= 'IMer',
	/*
		Returned after a request has succeded
	*/
	ACTION_PERFORMED	= 'IMok',
	/*
		All client <> protocol communication uses the MESSAGE what-code.
	*/
	MESSAGE				= 'IMme', 
	/*
		Endpoint management
	*/
	ADD_ENDPOINT		= 'IMae',
	REMOVE_ENDPOINT		= 'IMre',
	/*
		Settings management
	*/
	SETTINGS_TEMPLATE	= 'IMst',
	SETTINGS_UPDATED	= 'IMs0', // settings updated, notify protocol/client/etc
	
	/*
		Information about protocols
	*/
	GET_LOADED_PROTOCOLS	= 'IMgp',
	SERVER_BASED_CONTACT_LIST	= 'IMsl',
	
	/*
		Contact related messages
	*/
	GET_CONTACT_STATUS		='IMc0',
	GET_OWN_STATUSES		='IMc1',
	UPDATE_CONTACT_STATUS	= 'IMc2',
	GET_CONTACTS_FOR_PROTOCOL = 'IMc3',
	GET_ALL_CONTACTS		= 'IMc4',	// Request a list of all the contacts the server has, and statuses
	
	/*
		Deskbar icon related messages
	*/
	DESKBAR_ICON_CLICKED	= 'DBcl',
	REGISTER_DESKBAR_MESSENGER = 'DBrg',
	FLASH_DESKBAR			= 'DBfl',
	STOP_FLASHING			= 'DBst',
	
	/*
		Client autostart management.
		Replaced by per-client setting.
	*/
//	ADD_AUTOSTART_APPSIG	= 'Aaas',
//	REMOVE_AUTOSTART_APPSIG	= 'Raas'
	/*
		IM server status messages
	*/
	IS_IM_SERVER_SHUTTING_DOWN = 'IMsd'
};

/**
	The im_progress_message_type is intended to be attached
	to IM::PROGRESS messages in an int32 named "state" used
	to describe in what state the progress is.
	This can be used to display the status of a connect sequence
	or to show the status of a file transfer, for example.
	The states are not listed in any particular order, and don't
	have to be used in any particular order either. It's merely a
	help for the clients when they're displaying the message to
	the user.
*/
enum im_progress_state_type {
	impsResolving,
	impsConnecting,
	impsAuthenticating,
	impsSettingStatus,
	impsTransferringData,
	impsFinished
};

#define IM_SERVER_SIG "application/x-vnd.beclan.im_kit"

// Valid IM:status texts
#define ONLINE_TEXT		"Available"
#define AWAY_TEXT		"Away"
#define OFFLINE_TEXT	"Offline"
#define BLOCKED_TEXT	"Blocked"

};

#endif
