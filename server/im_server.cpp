#include "im_server.h"

#include "ContactManager.h"
#include "ConnectionStore.h"
#include "Private/Constants.h"
#include "ProtocolInfo.h"
#include "ProtocolManager.h"
#include "ProtocolSpecification.h"
#include "StatusIcon.h"
#include "StringSpecification.h"

#include "common/GenericStore.h"
#include "common/IMKitUtilities.h"

#include <libim/Constants.h>
#include <libim/Helpers.h>
#include <TranslationUtils.h>

#include <image.h>
#include <Roster.h>
#include <Path.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <stdio.h>
#include <Node.h>
#include <NodeMonitor.h>
#include <VolumeRoster.h>
#include <Volume.h>
#include <UTF8.h>
#include <libim/Helpers.h>
#include <Deskbar.h>
#include <Roster.h>
#include <kernel/fs_attr.h>
#include <Alert.h>
#include <String.h>
#include <Invoker.h>
#include <algorithm>
#include <Beep.h>
#include <PropertyInfo.h>
#include <Mime.h>
#include <storage/Resources.h>

#include <DeskbarIcon.h>

#ifdef ZETA
#include <locale/Locale.h>
#else
#define _T(str) (str)
#endif

#include "IMKitResources.h"

#include <signal.h>

using namespace IM;

//#pragma mark Constants

#define AUTOSTART_APPSIG_SETTING "autostart_appsig"

extern const char *kImConnectedSound = "IM Connected";
extern const char *kImDisconnectedSound	= "IM Disconnected";
extern const char *kImStatusOnlineSound = "IM Status: Available";
extern const char *kImStatusAwaySound = "IM Status: Away";
extern const char *kImStatusOfflineSound = "IM Status: Offline";

extern const char *kAppName = "im_server";

const char *kProtocolLoaderSig = "application/x-vnd.beclan.im_kit.ProtocolLoader";

const bigtime_t kQueryDelay = 5 * 1000 * 1000;	// 5 Seconds

//#pragma mark Functions

void _ERROR(const char *error, BMessage *msg) {
	LOG(kAppName, liHigh, error, msg);
};

void _ERROR( const char * error ) {
	_ERROR(error,NULL);
};

void _SEND_ERROR(const char *text, BMessage *msg) {
	if (msg->ReturnAddress().IsValid() == true ) {
		BMessage err(ERROR);
		err.AddString("error", text);
		msg->SendReply(&err);
	} else {
		// no recipient for message replies, write to stdout
		LOG(kAppName, liHigh, "ERROR: %s", text);
	};
};

//#pragma mark Constructor

/**
	Default constructor. (Starts People-query and ?) loads add-ons
*/
Server::Server(void)
	: BApplication(IM_SERVER_SIG),
	fIsQuitting(false),
	fCurProtocol(NULL),
	fProtocol(new ProtocolManager()),
	fContact(new ContactManager(this)) {

	LOG(kAppName, liHigh, "Starting im_server");
	
	BPath prefsPath;
	
	// Create our settings directories
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath, true, NULL) == B_OK) {
		BDirectory dir(prefsPath.Path());
		if (dir.InitCheck() == B_OK) {
			dir.CreateDirectory("im_kit", NULL);
			dir.CreateDirectory("im_kit/add-ons", NULL);
			dir.CreateDirectory("im_kit/add-ons/protocols", NULL);
			dir.CreateDirectory("im_kit/clients", NULL);
		};
	};
	
	InitSettings();
	
	BMessage settings;
	if (im_load_client_settings("im_server", &settings) == B_OK) {
		UpdateOwnSettings(settings);
	}
	
	LoadProtocols();

	_Init();
	
#if defined(ZETA)
	/* same badness as in DeskbarIcon.cpp */
	BPath path( "/boot/apps/Internet/IMKit/Language/Dictionaries/im_server" );
	if( path.InitCheck() == B_OK )
		be_locale.LoadLanguageFile( path.Path() );
#endif

	struct sigaction childExitedAction;
	childExitedAction.sa_handler = (__signal_func_ptr)&Server::ChildExited;
	childExitedAction.sa_mask = 0;
	childExitedAction.sa_flags = 0;
	childExitedAction.sa_userdata = this;
	sigaction(SIGCHLD, &childExitedAction, NULL);
};

/**
	Default destructor. Unloads add-ons and frees any aquired resources.
*/
Server::~Server(void) {
	LOG(kAppName, liDebug, "~Server start");
	
	GenericListStore<ProtocolInfo *> protocols = fProtocol->FindAll(new AllProtocolSpecification());
	for (GenericListStore<ProtocolInfo *>::Iterator pIt = protocols.Start(); pIt != protocols.End(); pIt++) {
		ProtocolInfo *info = (*pIt);
		
		if (fStatus[info->Signature()] != OFFLINE_TEXT) {
			// Non-offline protocol - set to offline		
			BMessage msg(MESSAGE);
			msg.AddInt32("im_what", STATUS_SET);
			msg.AddString("protocol", info->Signature());
			msg.AddString("status", OFFLINE_TEXT);
			msg.AddString("total_status", OFFLINE_TEXT);
			
			Broadcast(&msg);
			handleDeskbarMessage(&msg);
		};
	};

	StopAutostartApps();

	fProtocol->Unload();
	delete fProtocol;

	SetAllOffline();

	for (std::map<int, StatusIcon*>::iterator it = fStatusIcons.begin(); it != fStatusIcons.end(); ++it) {
		StatusIcon* icon = it->second;
		delete icon;
	};
	fStatusIcons.clear();

	LOG(kAppName, liDebug, "~Server end");
};

//#pragma mark Scripting Hooks

#define PROTOCOL_PROPERTY "Protocol"
#define PROTOCOLS_PROPERTY "Protocols"
#define STATUS_PROPERTY "Status"
static property_info sPropList[] =
{
	{PROTOCOL_PROPERTY,
		{0},
		{B_NAME_SPECIFIER, 0},
		"Gets the supported protocols",
	},
	{PROTOCOLS_PROPERTY,
		{B_GET_PROPERTY, 0},
		{B_DIRECT_SPECIFIER, 0},
		"Gets the supported protocols",
	},
	{STATUS_PROPERTY,
		{B_GET_PROPERTY, B_SET_PROPERTY, 0},
		{B_DIRECT_SPECIFIER, 0},
		"Get or set the status for a protocol",
	},
	0
};

status_t Server::GetSupportedSuites(BMessage *msg) {
	msg->AddString("suites", "suite/x.vnd-beclan.im_server"); 
	BPropertyInfo prop_info(sPropList);
	msg->AddFlat("messages", &prop_info); 
	return BApplication::GetSupportedSuites(msg);
};

/*
 * Most basic beos scripting capabilities. Examples using hey below.
 *
 * To get a list of supported protocols:
 *    hey application/x-vnd.beclan.im_kit get Protocols
 *
 * To get the current status of a specific protocol:
 *    hey application/x-vnd.beclan.im_kit get Status of Protocol icq
 *
 * To set the status of a specific protocol:
 *    hey application/x-vnd.beclan.im_kit set Status of Protocol icq to Available    
 */
BHandler *Server::ResolveSpecifier(BMessage *msg, int32 index, BMessage *specifier, int32 what, const char *property) {
	//TODO: Error handling.
	if (strcmp(property, PROTOCOLS_PROPERTY) == 0) {
		return this;
	} else if (strcmp(property, PROTOCOL_PROPERTY) == 0) {
		switch (what) {
			case B_NAME_SPECIFIER:{
				const char *name = specifier->FindString("name");
				
				fCurProtocol = NULL;
				fProtocol->FindFirst(new SignatureProtocolSpecification(name), &fCurProtocol);

				msg->PopSpecifier();
				return this;
			} break;
			default: {
			} break;
		};
	} else {
		if (strcmp(property, STATUS_PROPERTY) == 0) {
			switch (what) {
				case B_GET_PROPERTY:
				case B_SET_PROPERTY: {
					msg->PopSpecifier();
					return this;
				} break;
				default: {
				} break;
			};
		};
	};

	return BApplication::ResolveSpecifier(msg, index, specifier, what, property);
};

//#pragma mark BApplication Hooks

/**
*/
bool Server::QuitRequested(void) {
	fIsQuitting = true;

	while (CountWindows() > 0) {
		BMessenger msgr(WindowAt(0));
		if (msgr.IsValid()) msgr.SendMessage(B_QUIT_REQUESTED);
	};

	return true;
};

/**
*/
void Server::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case B_GET_PROPERTY: {
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			if (msg->GetCurrentSpecifier(&index, &specifier, &what, &property) != B_OK) {
				break;
			};
			
			//TODO: Error handling.
			if (strcmp(property, PROTOCOLS_PROPERTY) == 0) {
				BMessage reply(B_REPLY);

				GenericListStore<ProtocolInfo *> info = fProtocol->FindAll(new AllProtocolSpecification());
				
				for (GenericListStore<ProtocolInfo *>::Iterator it = info.Start(); it != info.End(); it++) {
					ProtocolInfo *info = (*it);
				
					reply.AddString("result", info->Signature());
				};

				msg->SendReply(&reply);
				
			} else {
				 if (strcmp(property, STATUS_PROPERTY) == 0) {
					BMessage reply(B_REPLY);
	
					if (fCurProtocol != NULL) {
						reply.AddString("result", fStatus[fCurProtocol->Signature()].c_str());
					};
					
					msg->SendReply(&reply);
					fCurProtocol = NULL;
				} else {
					BApplication::MessageReceived(msg);
				}
			};
		} break;
		
		case B_SET_PROPERTY: {
			int32 index;
			BMessage specifier;
			int32 what;
			const char *property;
			
			if (msg->GetCurrentSpecifier(&index, &specifier, &what, &property) != B_OK) {
				break;
			};
			
			const char *data = msg->FindString("data");
			
			// TODO: Needs to be updated to use instance_id (ie. Account, not Protocol)
			if (strcmp(property, STATUS_PROPERTY) == 0) {
				BMessage reply(B_REPLY);
				BMessage statusMessage(IM::MESSAGE);
				statusMessage.AddInt32("im_what", IM::SET_STATUS);
				statusMessage.AddString("status", data);

				if (fCurProtocol != NULL) {
					statusMessage.AddString("protocol", fCurProtocol->Signature());
				}
				
				Process(&statusMessage);
				msg->SendReply(&reply);
				fCurProtocol = NULL;
			} else {
				BApplication::MessageReceived(msg);
			}
		} break;
	
		// IM-Kit specified messages:		
		case GET_CONTACT_STATUS: {
			reply_GET_CONTACT_STATUS(msg);
		} break;
		
		case GET_OWN_STATUSES: {
			reply_GET_OWN_STATUSES(msg);
		} break;

		case UPDATE_CONTACT_STATUS: {
			reply_UPDATE_CONTACT_STATUS(msg);
		} break;
		
		case GET_CONTACTS_FOR_PROTOCOL: {
			reply_GET_CONTACTS_FOR_PROTOCOL(msg);
		} break;
			
		case REGISTER_DESKBAR_MESSENGER:
		case FLASH_DESKBAR:
		case STOP_FLASHING: {
			handleDeskbarMessage(msg);
		} break;
		
		case SERVER_BASED_CONTACT_LIST: {
			reply_SERVER_BASED_CONTACT_LIST(msg);
		} break;
		
		case SETTINGS_UPDATED: {
			handle_SETTINGS_UPDATED(msg);
		} break;
		
		case ADD_ENDPOINT: {
			BMessenger msgr;
			if (msg->FindMessenger("messenger", &msgr) == B_OK) {
				AddEndpoint( msgr );
				
				msg->SendReply( ACTION_PERFORMED );
			};
		} break;
		
		case REMOVE_ENDPOINT: {
			BMessenger msgr;
			if (msg->FindMessenger("messenger", &msgr) == B_OK) {
				RemoveEndpoint( msgr );
				
				msg->SendReply( ACTION_PERFORMED );
			};
		} break;
		
		case GET_LOADED_PROTOCOLS: {
			reply_GET_LOADED_PROTOCOLS(msg);
		} break;
		
		case GET_ALL_CONTACTS: {
			reply_GET_ALL_CONTACTS(msg);
		} break;
		
		case IS_IM_SERVER_SHUTTING_DOWN: {
			BMessage reply(IS_IM_SERVER_SHUTTING_DOWN);
			reply.AddBool("isShuttingDown", fIsQuitting);
			
			msg->SendReply( &reply );
		} break;
		
		case ERROR: {
			Broadcast( msg );
		} break;
			
		case MESSAGE: {
			Process(msg);
		} break;
		
		case IM::Private::PROTOCOL_STARTED: {
			const char *instanceID;
			const char *signature;
			const char *friendlySignature;
			uint32 capabilities;
			uint32 encoding;
			BMessage settings;
			BMessenger msgr;
					
			if (msg->FindString("instance_id", &instanceID) != B_OK) return;
			if (msg->FindString("signature", &signature) != B_OK) return;
			if (msg->FindString("friendly_signature", &friendlySignature) != B_OK) return;
			if (msg->FindInt32("capabilities", (int32 *)&capabilities) != B_OK) return;
			if (msg->FindInt32("encoding", (int32 *)&encoding) != B_OK) return;
			if (msg->FindMessage("template", &settings) != B_OK) return;
			if (msg->FindMessenger("messenger", &msgr) != B_OK) return;

			ProtocolInfo *info = NULL;
			if (fProtocol->FindFirst(new InstanceProtocolSpecification(instanceID), &info) == false) {
				LOG(kAppName, liHigh, "Got a PROTOCOL_STARTED message for an unexpected protocol: %s (%s)", signature, instanceID);
				return;
			}
	
			info->Signature(signature);
			info->FriendlySignature(friendlySignature);
			info->Capabilities(capabilities);
			info->SettingsTemplate(settings);
			info->Messenger(new BMessenger(msgr));
			
			BMessage add(MESSAGE);
			add.AddInt32("im_what", REGISTER_CONTACTS);

//			XXX
			GetContactsForProtocol(signature, &add);
			info->Process(&add);
		
			// Inform everyone of the new protocol
			BMessage changed(LOADED_PROTOCOLS_CHANGED);
			Broadcast(&changed);
		} break;
		
		case IM::Private::PROTOCOL_COULD_NOT_START:
		case IM::Private::PROTOCOL_KILLED:
		case IM::Private::PROTOCOL_STOPPED: {
			// Remove the protocol info
			const char *instanceID;
			
			if (msg->FindString("instance_id", &instanceID) != B_OK) return;
			
			ProtocolInfo *info = NULL;
			if (fProtocol->FindFirst(new InstanceProtocolSpecification(instanceID), &info) == false) {
				LOG(kAppName, liHigh, "Got a PROTOCOL_STOPPED / PROTOCOL_KILLED / PROTOCOL_COULD_NOT_START message for a protocol we don't know about: %s", instanceID);
				return;
			};
		
			info->Stop();

			// Update our status and those of our contacts
			ProtocolOffline(info->Signature());

			// Tell everyone the protocol has gone offline
			BMessage offline(MESSAGE);
			offline.AddInt32("im_what", STATUS_SET);
			offline.AddString("protocol", info->Signature());
			offline.AddString("status", OFFLINE_TEXT);
			offline.AddString("total_status", TotalStatus());
				
			handleDeskbarMessage(&offline);

			// Broadcast the message
			BMessage changed(LOADED_PROTOCOLS_CHANGED);		
			Broadcast(&changed);
			
			if ((msg->what == IM::Private::PROTOCOL_KILLED) && (fIsQuitting == false)) {
				// Restart the protocol
				fProtocol->RestartProtocols(new InstanceProtocolSpecification(instanceID));
			};
			
			if (msg->what == IM::Private::PROTOCOL_COULD_NOT_START) {
				const char *reason = NULL;
				if (msg->FindString("reason", &reason) != B_OK) reason = NULL;
				
				LOG(kAppName, liHigh, "Got a PROTOCOL_COULD_NOT_START message for %s: %s", instanceID, (reason != NULL) ? reason : "No reason obtained");
				info->Stop();
			};
		} break;
		
		// Other messages
		default: {
			BApplication::MessageReceived(msg);
		} break;
	}
}

//#pragma mark ContactListener Hooks

void Server::ContactAdded(Contact *contact) {
	char connection[512];

	// Iterate the Contact's connections and notify the Protocol of the Contact's addition
	for (int i = 0; contact->ConnectionAt(i, connection) == B_OK; i++) {
		Connection conn(connection);
		
		ProtocolInfo *info = NULL;
		if (fProtocol->FindFirst(new SignatureProtocolSpecification(conn.Protocol()), &info) == true) {
			BMessage add(MESSAGE);
			add.AddInt32("im_what", REGISTER_CONTACTS);
			add.AddString("id", conn.ID());

			info->Process(&add);			
		};
	};
};

void Server::ContactModified(Contact *contact, ConnectionStore *oldConnections, ConnectionStore *newConnections) {
	bool conModified = false;

	// Find removed items
	for (ConnectionStore::Iterator oldConIt = oldConnections->Start(); oldConIt != oldConnections->End(); oldConIt++) {
		Connection con = (*oldConIt);

		// If we can't find the Connection in the new list it must have been removed - notify protocols
		if (newConnections->Contains(con) == false) {
			conModified = true;
		
			ProtocolInfo *info = NULL;
			if (fProtocol->FindFirst(new SignatureProtocolSpecification(con.Protocol()), &info) == true) {
				BMessage remove(MESSAGE);
				remove.AddInt32("im_what", UNREGISTER_CONTACTS);
				remove.AddString("id", con.ID());
			
				info->Process(&remove);	
				
				// XXX Should remove it from the status list as well
				std::map<std::string, std::string>::iterator iter = fStatus.find(con.ID());
				if ( iter != fStatus.end() )
					fStatus.erase( iter );
			};
		};
	};

	
	// Find new items
	// TODO: This seems to cause a crash on quit when this for() is enabled. Not sure of the exact cause -- slaad
	for (ConnectionStore::Iterator newConIt = newConnections->Start(); newConIt != newConnections->End(); newConIt++) {
		Connection con = (*newConIt);

		// If we can't find the Connection in the old list it must have been added - notify protocols
		if (oldConnections->Contains(con) == false) {
			conModified = true;
		
			ProtocolInfo *info = NULL;
			if (fProtocol->FindFirst(new SignatureProtocolSpecification(con.Protocol()), &info) == true) {
				// protocol loaded, register connection
				BMessage remove(MESSAGE);
				remove.AddInt32("im_what", REGISTER_CONTACTS);
				remove.AddString("id", con.ID());
				
				info->Process(&remove);
			};
		};
	};
	
	if (conModified == true) {
		UpdateContactStatusAttribute(*contact);
	};
};

void Server::ContactRemoved(Contact *contact, ConnectionStore *oldConnections) {
	bool conModified = false;

	for (ConnectionStore::Iterator it = oldConnections->Start(); it != oldConnections->End(); it++) {
		Connection conn = (*it);
		bool canRemove = false;
	
		// Ensure no other Contacts use this Connection
		GenericListStore<ContactCachedConnections *> other = fContact->FindAll(new ConnectionContactSpecification(conn));
		if (other.CountItems() == 0) {
			canRemove = true;
		};
		if ((other.CountItems() == 1) && (contact == *other.Start())) {
			canRemove = true;
		};

		if (canRemove == true) {
			conModified = true;

			ProtocolInfo *info = NULL;
			if (fProtocol->FindFirst(new SignatureProtocolSpecification(conn.Protocol()), &info) == true) {
				BMessage remove(MESSAGE);
				remove.AddInt32("im_what", UNREGISTER_CONTACTS);
				remove.AddString("id", conn.ID());
				
				info->Process(&remove);
			};
			
//				// XXX remove from fStatus too
				std::map<std::string, std::string>::iterator iter = fStatus.find(conn.ID());
				if ( iter != fStatus.end() )
					fStatus.erase( iter );
		};
	};
	
	if (conModified == true) {
		UpdateContactStatusAttribute(*contact);
	};
};


ProtocolManager* Server::GetProtocolLoader() const
{
	return fProtocol;
}

//#pragma mark Private

void Server::_Init(void) {
	_UpdateStatusIcons();
};

void Server::_UpdateStatusIcons(void) {
	image_info info;	
	if (our_image(info) != B_OK) {
		return;
	};

	BFile file(info.name, B_READ_ONLY);
	if (file.InitCheck() < B_OK) {
		return;
	};

	BResources resources(&file);
#if defined(__HAIKU__)
	if (resources.InitCheck() < B_OK) {
		return;
	};
#endif

	size_t size = 0;
	const void* data = NULL;

#if defined(__HAIKU__)
	LOG("im_server", liDebug, "Caching HVIF icons...");

	for (int i = kAvailableStatusIcon; i <= kOfflineStatusIcon; i++) {
		StatusIcon *icon = new StatusIcon();

		data = resources.LoadResource(B_VECTOR_ICON_TYPE, i, &size);
		if (data != NULL) {
			icon->SetVectorIcon(data, size);
		};

		if (icon->IsEmpty() == false) {
			fStatusIcons.insert(std::pair<int, StatusIcon*>(i, icon));
		};
	}
#endif

#if defined(BEOS) || defined(ZETA)
	LOG("im_server", liDebug, "Caching bitmap icons...");

	for (int i = kAvailableStatusIcon; i <= kOfflineStatusIcon; i++) {
		StatusIcon *icon = new StatusIcon();

		// Mini icon
		data = resources.LoadResource(B_MINI_ICON_TYPE, kAvailableStatusIconSmall + i, &size);
		if (data != NULL) {
			icon->SetMiniIcon(data, size);
		};

		// Large icon
		data = resources.LoadResource(B_LARGE_ICON_TYPE, kAvailableStatusIconLarge + i, &size);
		if (data != NULL) {
			icon->SetLargeIcon(data, size);
		};

		if (icon->IsEmpty() == false) {
			fStatusIcons.insert(pair<int, StatusIcon*>(i, icon));
		};
	}
#endif
}

/**
	Install deskbar icon.
*/
void Server::_InstallDeskbarIcon(void) {
	entry_ref ref;
	bool valid = false;

	if (be_roster->FindApp(DESKBAR_ICON_SIG, &ref) == B_OK) {
		valid = true;
	} else {
		// Try with a query and take the first result
		BVolumeRoster vroster;
		BVolume volume;
		char volName[B_FILE_NAME_LENGTH];

		vroster.Rewind();

		while (vroster.GetNextVolume(&volume) == B_OK) {
			if ((volume.InitCheck() != B_OK) || !volume.KnowsQuery())
				continue;

			volume.GetName(volName);
			LOG(kAppName, liDebug, "_InstallDeskbarIcon: Trying with a query on %s", volName);

			BQuery *query = new BQuery();
			query->SetPredicate("(BEOS:APP_SIG==\""DESKBAR_ICON_SIG"\")");
			query->SetVolume(&volume);
			query->Fetch();

			if (query->GetNextRef(&ref) == B_OK) {
				valid = true;
				break;
			};

			LOG(kAppName, liHigh, "Unable to find Deskbar icon - waiting before retrying");
			snooze(kQueryDelay);

			if (query->GetNextRef(&ref) == B_OK) {
				valid = true;
				break;
			};
		};
	};

	if (valid == true) {
		BDeskbar deskbar;
		deskbar.AddItem(&ref);
	};
}

/**
	Load protocol add-ons and init them
*/
status_t Server::LoadProtocols(void) {
	BDirectory settingsDir; // base directory for protocol settings
	BDirectory addonsDir; // directory for protocol addons
	status_t rc_set;
	status_t rc_sys;
	status_t rc_user;
	
	// STEP 1: Check if we can access settings for the protocols!
	BPath path;
	if ((rc_set = find_directory(B_USER_SETTINGS_DIRECTORY,&path,true)) != B_OK ||
		(rc_set = path.Append("im_kit/add-ons/protocols")) != B_OK ||
		(rc_set = settingsDir.SetTo(path.Path())) != B_OK) {
		
		// we couldn't access the settings directory for the protocols!
		LOG(kAppName, liHigh, "Cannot access protocol settings directory: %s, error 0x%lx (%s)!", path.Path(), rc_set, strerror(rc_set));
		return rc_set;
	};

	// Okies, we've been able to access our critical dirs, so now we should be sure we can load any addons that are there
	fProtocol->Unload();
	
	// STEP 2a: Check if we can access the system add-on directory for the protocols!
	BPath commonAddon;
	if ((rc_sys = find_directory(B_COMMON_ADDONS_DIRECTORY, &commonAddon, true)) != B_OK ||
		(rc_sys = commonAddon.Append("im_kit/protocols")) != B_OK ||
		(rc_sys = addonsDir.SetTo(commonAddon.Path())) != B_OK) {

		// we couldn't access the addons directory for the protocols!
		LOG(kAppName, liHigh, "Cannot access system protocol addon directory: %s, error 0x%lx (%s)!", commonAddon.Path(), rc_sys, strerror(rc_sys));
	} else {
		fProtocol->LoadFromDirectory(addonsDir, settingsDir);
	};

	// STEP 2b: Check if we can access the user add-on directory for the protocols!
	BPath userAddon;
	if ((rc_user = find_directory(B_USER_ADDONS_DIRECTORY, &userAddon, true)) != B_OK ||
		(rc_user = userAddon.Append("im_kit/protocols")) != B_OK ||
		(rc_user = addonsDir.SetTo(userAddon.Path())) != B_OK) {
		// we couldn't access the addons directory for the protocols!
		LOG(kAppName, liHigh, "Cannot access user protocol addon directory: %s, error 0x%lx (%s)!", userAddon.Path(), rc_user, strerror(rc_user));
	} else {
		if (userAddon != commonAddon) {
			fProtocol->LoadFromDirectory(addonsDir, settingsDir);
		} else {
			LOG(kAppName, liMedium, "User addon path is the same as system addon path - skipping");
		};
	};
	
	if ((rc_sys != B_OK) && (rc_user != B_OK)) {
		return B_ERROR;
	} else {
		LOG(kAppName, liMedium, "All add-ons loaded.");
		return B_OK;
	};
};

/**
	Add a listener endpoint that will receive all messages
	broadcasted from the im_server
*/
void Server::AddEndpoint(BMessenger msgr) {
	LOG(kAppName, liDebug, "Endpoint added");
	fMessengers.push_back(msgr);
};

/**
	Remove a listener endpoint.
*/
void
Server::RemoveEndpoint(BMessenger msgr)
{
	LOG(kAppName, liDebug, "Endpoint removed");
	fMessengers.remove(msgr);
}

/**
	Process an IM_MESSAGE BMessage, broadcasting or posting to protocols
	as needed to perform the requested operation. Called from MessageReceived()
	
	@param msg The message to process
*/
void Server::Process(BMessage * msg) {
	int32 im_what = -1;
	
	if ( msg->FindInt32("im_what",&im_what) != B_OK ) {
		// Malformed message, skip
		_ERROR("Server::Process(): Malformed message", msg);
		return;
	};
	
	switch (im_what) {
		// messages to protocols
		case GET_CONTACT_LIST:
		case SET_STATUS:
		case SEND_MESSAGE:
		case GET_CONTACT_INFO:
		case SEND_AUTH_ACK:
		case USER_STARTED_TYPING:
		case USER_STOPPED_TYPING:
		case SPECIAL_TO_PROTOCOL:
		case SERVER_LIST_ADD_CONTACT:
		case SERVER_LIST_REMOVED_CONTACT: {
			MessageToProtocols(msg);
		} break;
		
		// messages from protocols
		case STATUS_SET:
		case MESSAGE_SENT:
		case MESSAGE_RECEIVED:
		case STATUS_CHANGED:
		case CONTACT_LIST:
		case CONTACT_INFO:
		case CONTACT_AUTHORIZED:
		case CONTACT_STARTED_TYPING:
		case CONTACT_STOPPED_TYPING:
		case SET_BUDDY_ICON:
		case PROGRESS:
		case SPECIAL_FROM_PROTOCOL: {
			MessageFromProtocols(msg);
		} break;
		// authorization requests
		case AUTH_REQUEST: {
			// this should probably be in the client at a later time.
			BString authProtocol;
			BString authUIN;
			BString authMessage;

			BString authText;
			
			msg->FindString("protocol", &authProtocol);
			msg->FindString("id", &authUIN);
			msg->FindString("message", &authMessage);
			
			authText  = "Authorization request from ";
			authText += authUIN;
			authText += " :\n\n";
			authText += authMessage;
			authText += "\n\nDo you want to accept it?";

			BMessage * authReply = new BMessage(MESSAGE);
			authReply->AddInt32("im_what", SEND_AUTH_ACK);
			authReply->AddString("protocol", authProtocol.String());
			authReply->AddString("id", authUIN.String()); 

			BInvoker * authInv = new BInvoker(authReply, this);
			
			BAlert * authAlert = new BAlert("Auth Request Alert", authText.String(), "Yes", "No", NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
			authAlert->Go(authInv);
		} break;
		default: {
			// Unknown im_what opcode, skip and report
			_ERROR("Unknown im_what code", msg);
		} break;
	};
};

/**
	Send a BMessage to all registered BMessengers after converting 'message' encoding if needed.
	
	@param msg The message to send
*/
void Server::Broadcast(BMessage * msg) {
	// add friendly protocol name if applicable
	const char * protocol;
	for (int i = 0; msg->FindString("protocol", i, &protocol) == B_OK; i++) {
		const char *friendly = "<invalid protocol>";
		ProtocolInfo *info = NULL;
		if (fProtocol->FindFirst(new SignatureProtocolSpecification(protocol), &info) == true) {
			friendly = info->FriendlySignature();
		};

		msg->AddString("userfriendly", friendly);
	};
	// done adding fancy names
	
	std::list<BMessenger>::iterator i;
	
	for (i = fMessengers.begin(); i != fMessengers.end(); i++) {
		BMessenger msgr = (*i);
		
		if (msgr.IsTargetLocal() == false) {
			msgr.SendMessage(msg);
		} else {
			_ERROR("Broadcast(): messenger local");
		};
	};
};

/**
	Select the 'best' protocol for sending a message to contact
*/
status_t Server::SelectConnection(BMessage *msg, Contact &contact) {
	LOG(kAppName, liHigh, "SelectConnection(%4.4s, %s) entered", &msg->what, ((const entry_ref *)contact)->name);

	char connection[255];
	const char *protocol = msg->FindString("protocol");
	const char *id = msg->FindString("id");

	// first of all, check if source of last message is still online
	// if it is, we use it.
	if ((fPreferredConnection[contact].length() > 0) && (fPreferredConnection[contact].length() < 100)) {

		strncpy(connection, fPreferredConnection[contact].c_str(), sizeof(connection));
		connection[sizeof(connection)-1] = 0;
			
		if ((fStatus[connection].length() > 0) && (fStatus[connection] != OFFLINE_TEXT)) {
			Connection con(connection);
			if (fStatus[con.Protocol()] != OFFLINE_TEXT) {
				LOG(kAppName, liDebug, "Using preferred connection %s", connection);
				
				if (protocol == NULL) {
					msg->AddString("protocol", con.Protocol());
				};
				if (id == NULL) {
					msg->AddString("id", con.ID());
				};
				
				return B_OK;
			};
		};
		
		LOG(kAppName, liDebug, "Preferred connection [%s] not online", connection);
	};
	
	// look for an online protocol
	if ((protocol != NULL) && (id != NULL)) {
		// all set
		return B_OK;
	};
	
	for (int i = 0; contact.ConnectionAt(i, connection) == B_OK; i++) {
		std::string curr = connection;
		Connection con(connection);		

		if (protocol != NULL) {	
			if (con.Protocol() != protocol) {
				// protocol selected and this is not it. skip.
				continue;
			};
		};
		
		if ((fStatus[curr].length() > 0) && (fStatus[curr] != OFFLINE_TEXT)) {
			LOG(kAppName, liHigh, "This connection is online (%s)", curr.c_str());

			if (fStatus[con.Protocol()] != OFFLINE_TEXT) {
				// make sure WE'RE online on this protocol too
				if (protocol == NULL) {
					msg->AddString("protocol", con.Protocol());
				};
				msg->AddString("id", con.ID());

				LOG(kAppName, liDebug, "Using online connection %s", con.String());
				return B_OK;
			};
		};
	};

	// Obtain a list off Offline capable Protocols	
	GenericListStore<ProtocolInfo *> protocols = fProtocol->FindAll(new CapabilityProtocolSpecification(Protocol::OFFLINE_MESSAGES));

	for (GenericListStore<ProtocolInfo *>::Iterator pIt = protocols.Start(); pIt != protocols.End(); pIt++) {
		ProtocolInfo *info = (*pIt);

		// Check if contact has a connection for this protocol
		if (contact.FindConnection(info->Signature(), connection) == B_OK) {

			// Make sure we're online with this protocol
			if (fStatus[info->Signature()] != OFFLINE_TEXT) {
				LOG(kAppName, liDebug, "Using offline connection %s", connection);
				Connection con(connection);

				if (protocol == NULL) {
					msg->AddString("protocol", con.Protocol());
				};
				if (id == NULL) {
					msg->AddString("id", con.ID());
				};
				
				return B_OK;
			};
		};
	};	
	
	// No matching Protocol
	return B_ERROR;
};

/**
	Perform a number of sanity checks on a message, returning true if it's ok
*/
bool Server::IsMessageOk(BMessage * msg) {
	const char *str = NULL;
	
	if (msg->FindString("protocol", &str) == B_OK) {
		if ((strlen(str) == 0) || (strlen(str) > 100)) {
			LOG(kAppName, liHigh, "IsMessageOk(): invalid protocol present");
		};
	};
	
	if (msg->FindString("id", &str) == B_OK) {
		if ((strlen(str) == 0) || (strlen(str) > 100)) {
			LOG(kAppName, liHigh, "IsMessageOk(): invalid id present");
		};
	};
	
	return true;
};

/**
	Forward message from client-side to protocol-side
*/
void Server::MessageToProtocols(BMessage * msg) {
	if (IsMessageOk(msg) == false) {
		LOG(kAppName, liHigh, "Bad message in MessageToProtocols()");
		return;
	};
	
	entry_ref entry;
	
	if (msg->FindRef("contact", &entry) == B_OK) {
		// contact present, select protocol and ID
		Contact contact(entry);
		
		if ((contact.InitCheck() != B_OK) || !contact.Exists()) {
			// invalid target
			_SEND_ERROR("Invalid target, no such contact", msg);
			return;
		};
		
		if (SelectConnection(msg, contact) != B_OK) {
			// No available connection, can't send message!
			LOG(kAppName, liHigh, "Can't send message, no possible connection");
			
			// send ERROR message here..
			BMessage error(ERROR);
			error.AddRef("contact", contact);
			error.AddString("error", "Can't send message, no available connections. Go online!");
			error.AddMessage("message", msg);
				
			Broadcast(&error);
			return;
		};
				
		if (fStatus[msg->FindString("protocol")] == OFFLINE_TEXT) {
			// selected protocol is offline, impossible to send message
			BString error_str;
			error_str << "Not connected to selected protocol [";
			error_str << msg->FindString("protocol");
			error_str << "], cannot send message";
			
			_ERROR(error_str.String(), msg);
			
			BMessage err(IM::ERROR);
			err.AddRef("contact", contact);
			err.AddString("error", error_str.String());
			
			Broadcast(&err);
			
			return;
		};
		
		if (msg->FindString("id") == NULL) {
			// add protocol-specific ID from Contact if not present
			char connection[255];
			
			if (contact.FindConnection(msg->FindString("protocol"),connection) != B_OK) {
				_ERROR("Couldn't get connection for protocol", msg);
				return;
			};
			
			const char *id = NULL;
			
			// TODO: Investigate this - shouldn't we be using IM:Connection for parsing?
			for (int i = 0; connection[i]; i++) {
				if (connection[i] == ':') {
					id = &connection[i+1];
					break;
				};
			};
			
			if (id == NULL) {
				_ERROR("Couldn't get ID from connection", msg);
				return;
			};
			
			msg->AddString("id", id);
		};
	} // done selecting protocol and ID
	
	// copy message so we can broadcast it later, with data intact
	BMessage client_side_msg(*msg);
	const char *signature = NULL;
	const char *instance_id = NULL;
	
	msg->FindString("instance_id", &instance_id);
	msg->FindString("protocol", &signature);
	
	// TODO: Temporary measure: signature check should be replaced by instance_id only
	if ((signature == NULL) && (instance_id == NULL)) {
		// no protocol specified, send to all?
		LOG(kAppName, liLow, "No protocol specified");
		
		int32 im_what = B_ERROR;
		msg->FindInt32("im_what", &im_what);
		
		switch (im_what) {
			// send these messages to all loaded protocols
			case SET_STATUS: {
				LOG(kAppName, liLow, "SET_STATUS - Sending to all protocols");
				
				if (fProtocol->MessageProtocols(new AllProtocolSpecification(), msg) != B_OK) {
					_ERROR("One or more protocols did not receive our SET_STATUS");
				};
			} break;
			
			default: {
				_ERROR("Invalid message", msg);
			} return;
		};
	} else {
		ProtocolInfo *info = NULL;
		bool valid = true;
		
		if ((signature != NULL) && (fProtocol->FindFirst(new SignatureProtocolSpecification(signature), &info) == false)) {
			valid = false;
		};
		
		if ((instance_id != NULL) && (fProtocol->FindFirst(new InstanceProtocolSpecification(instance_id), &info) == false)) {
			valid = false;
		};
			
		if (valid == false) {
			// invalid protocol, report and skip
			_ERROR("Protocol not loaded or not installed", msg);			
			_SEND_ERROR("Protocol not loaded or not installed", msg);
			return;
		}
	
		int32 charset = info->Encoding();
		if (charset != 0xffff) {
			// convert to desired charset		
			msg->AddInt32("charset", charset );
			client_side_msg.AddInt32("charset", charset);
			
			type_code _type;
			char *name = NULL;
			int32 count = -1;

#if B_BEOS_VERSION > B_BEOS_VERSION_5			
			for (int i=0; msg->GetInfo(B_STRING_TYPE, i, (const char **)&name, &_type, &count) == B_OK; i++)
#else
			for (int i=0; msg->GetInfo(B_STRING_TYPE, i, &name, &_type, &count) == B_OK; i++)
#endif
			{
				// get string names
				for (int x = 0; x < count; x++) {
					// replace all matching strings
					const char *data = msg->FindString(name, x);
					
					int32 src_len = strlen(data);
					int32 dst_len = strlen(data)*5;
					int32 state = 0;
					
					char *new_data = new char[dst_len];

					if (convert_from_utf8(charset, data, &src_len, new_data, &dst_len, &state) == B_OK)	{
						new_data[dst_len] = 0;
						
						msg->ReplaceString(name, x, new_data);
					};
					
					delete[] new_data;
				};
			};
		}; // done converting charset
		
		
		if (info->Process(msg) != B_OK) {
			_SEND_ERROR("Protocol reports error processing message", msg);
			return;
		};
	};
	
	// broadcast client_side_msg, since clients want utf8 text, and that has
	// been replaced in msg with protocol-specific data
	Broadcast(&client_side_msg);
};


/**
	Handle a message coming from protocol-side to client-side
*/
void Server::MessageFromProtocols(BMessage *msg) {
	if (IsMessageOk(msg) == false) {
		LOG(kAppName, liHigh, "Bad message in MessageFromProtocols()");
		return;
	};

	const char *protocol = NULL;
	if (msg->FindString("protocol", &protocol) != B_OK) {
		LOG(kAppName, liHigh, "Got a message with no protocol!");
		return;
	};

	const char *instance_id = NULL;
	if (msg->FindString("instance_id", &instance_id) != B_OK) {
		LOG(kAppName, liHigh, "Got a protocol message with no instance!");
		return;
	};
	
	ProtocolInfo *info = NULL;
	if (fProtocol->FindFirst(new AndProtocolSpecification(new SignatureProtocolSpecification(protocol), new InstanceProtocolSpecification(instance_id)), &info) == false) {
		LOG(kAppName, liHigh, "Got a protocol message which cannot be matched to an existing account");
		return;
	};		
	
	// convert strings to utf8
	int32 charset;
	if (msg->FindInt32("charset", &charset) == B_OK) {
		// charset present, convert all strings
		type_code _type;
		char *name = NULL;
		int32 count = -1;

#if B_BEOS_VERSION > B_BEOS_VERSION_5
		for (int i = 0; msg->GetInfo(B_STRING_TYPE, i, (const char **)&name, &_type, &count) == B_OK; i++)
#else
		for (int i = 0; msg->GetInfo(B_STRING_TYPE, i, &name, &_type, &count) == B_OK; i++)
#endif
		{
			// Get string names
			for (int x = 0; x < count; x++) {
				// Replace all matching strings
				const char *data = msg->FindString(name, x);
				int32 src_len = strlen(data);
				int32 dst_len = strlen(data)*5;
				int32 state = 0;

				char *new_data = (char *)calloc(dst_len + 1, sizeof(char));

				if (convert_to_utf8(charset, data, &src_len, new_data, &dst_len, &state) == B_OK) {
					new_data[dst_len] = 0;
					msg->ReplaceString(name, x, new_data);
				};

				free(new_data);;
			};
		};
	};
	// done converting

	int32 im_what = B_ERROR;
	msg->FindInt32("im_what", &im_what);

	entry_ref testc;

	if (msg->FindRef("contact", &testc) == B_OK) {
		_ERROR("contact already present in message supposedly from protocol", msg);
		return;
	};

	// Find out which contact this message originates from
	ContactCachedConnections *contact = NULL;
	const char * id = msg->FindString("id");

	if (id != NULL) {
		// ID present, find out which Contact it belongs to
		if (protocol == NULL) {
			// Malformed message. report and skip.
			_ERROR("Malformed message in Server::Process", msg);
			return;
		};

		Connection connection(protocol, info->AccountName(), id);
		if ((fContact->FindFirst(new ConnectionContactSpecification(connection), &contact) == false) || (contact->InitCheck() != B_OK)) {
			contact = fContact->CreateContact(connection, id);

			BMessage connection(MESSAGE);
			connection.AddInt32("im_what", REGISTER_CONTACTS);
			connection.AddString("id", id);

			ProtocolInfo *info = NULL;
			if (fProtocol->FindFirst(new SignatureProtocolSpecification(protocol), &info))
				info->Process(&connection);
		};
		
		// add all matching contacts to message
		GenericListStore<ContactCachedConnections *> contacts = fContact->FindAll(new ConnectionContactSpecification(connection));
		
		for (GenericListStore<ContactCachedConnections *>::Iterator iter = contacts.Start(); iter != contacts.End(); iter++) {
			Contact *contact = (*iter);

			if (im_what == MESSAGE_RECEIVED) {
				// Message received from contact, store the connection in fPreferredProtocol
				char status[256];
				contact->GetStatus(status, sizeof(status));

				if (strcmp(status, BLOCKED_TEXT) == 0) {
					// Contact blocked, dropping message!
					LOG(kAppName, liHigh, "Dropping message from blocked contact [%s:%s]", protocol, id);
				} else {
					msg->AddRef("contact", *contact);

					// XXX
					if ( fPreferredConnection[*contact] != connection.String()) {
						// set preferred connection to this one if it's no already that
						fPreferredConnection[*contact] = connection.String();
						LOG(kAppName, liLow, "Setting preferred connection for contact to %s", connection.String() );
					};
				};
			} else {
				// Always add contact to other messages
				msg->AddRef("contact", *contact);
			};
		};
	};

	if (im_what == CONTACT_INFO) {
		// Get contact information
		const char *first_name = msg->FindString("first name");
		const char *middle_name = msg->FindString("middle name");
		const char *last_name = msg->FindString("last name");
		const char *fname = msg->FindString("full name");
		const char *email = msg->FindString("email");
		const char *nick = msg->FindString("nick");
		const char *url = msg->FindString("url");
		const char *bday = msg->FindString("birthday");

		// Compose contact's full name
		BString full_name(fname);
		if (full_name == "") {
			if (first_name)
				full_name.Append(first_name);
			if (first_name && middle_name)
				full_name.Append(" ");
			if (middle_name)
				full_name.Append(middle_name);
			if (first_name && last_name)
				full_name.Append(" ");
			if (last_name)
				full_name.Append(last_name);
		}

		// Update contact information (the attributes will be set only when not already set)
		ContactHandle handle = contact->Handle();
		Contact clientContact(handle.entry);
		clientContact.SetName(full_name.String());
		clientContact.SetNickname(nick);
		clientContact.SetEmail(email);
		clientContact.SetURL(url);
		clientContact.SetBirthday(bday);
	};

	if (im_what == SET_BUDDY_ICON) {
		LOG(kAppName, liHigh, "Got a buddy icon from a protocol!");
		const uchar *data;
		int32 bytes = -1;
		entry_ref ref;

		if (msg->FindRef("contact", &ref) != B_OK) {
			LOG(kAppName, liHigh, "No contact in buddy message.");
			return;
		};

		if (msg->FindData("icondata", B_RAW_TYPE, (const void **)&data, &bytes) !=  B_OK) {
			LOG(kAppName, liHigh, "No icondata in buddy message.");
		} else {
			// Fetch the protocol again... there's a weird memory corruption error happening
			const char *protocol = NULL;
			if (msg->FindString("protocol", &protocol) != B_OK) return;

			Contact contact(ref);

			BMallocIO buffer;
			buffer.WriteAt(0, data, bytes);
			BBitmap *icon = BTranslationUtils::GetBitmap(&buffer);

			if (icon == NULL) {
				LOG(kAppName, liHigh, "Unable to decode buddy icon.");
			} else {
				LOG(kAppName, liDebug, "Setting %s's icon to be %p", protocol, icon);

				status_t ret = contact.SetBuddyIcon(protocol, icon);
				if ((ret == B_OK) && (contact.GetBuddyIcon("general") == NULL)) {
					LOG(kAppName, liDebug, "Also setting the general icon, since none was set");
					ret = contact.SetBuddyIcon("general", icon);
				};

				BMessage update(MESSAGE);
				update.AddInt32("im_what", BUDDY_ICON_UPDATED);
				update.AddRef("contact", &ref);

				Broadcast(&update);
			};
		};

		return;
	};

	if ((im_what == STATUS_CHANGED) && (protocol != NULL) && (id != NULL)) {
		// update status list on STATUS_CHANGED
		UpdateStatus(msg);
	};

	if ((im_what == STATUS_SET) && (protocol != NULL)) {
		// own status set for protocol, register the id's we're interested in etc
		handle_STATUS_SET(msg);
	};

	if ((im_what == CONTACT_AUTHORIZED) && (protocol != NULL)) {
		LOG(kAppName, liLow, "Creating new contact on authorization. ID : %s", id);
	} else {
		// send it
		Broadcast(msg);
	};
};

/**
	Update the status of contacts in a STATUS_CHANGED message. Set im_server internal
	status and calls UpdateContactStatusAttribute to update IM:status attribute
	
	@param msg 			The message is a STATUS_CHANGED message.
*/
void Server::UpdateStatus(BMessage * msg) {
	const char *id = NULL;
	if (msg->FindString("id", &id) != B_OK) {
		_ERROR("ERROR: No ID on UpdateStatus message", msg);
		return;
	};

	const char *protocol = NULL;
	if (msg->FindString("protocol", &protocol) != B_OK) {
		_ERROR("ERROR: Malformed UpdateStatus message", msg);
		return;
	};
	
	const char *instance_id = NULL;
	if (msg->FindString("instance_id", &instance_id) != B_OK) {
		LOG(kAppName, liHigh, "Got a protocol message with no instance!");
		return;
	};
	
	ProtocolInfo *info = NULL;
	if (fProtocol->FindFirst(new AndProtocolSpecification(new SignatureProtocolSpecification(protocol), new InstanceProtocolSpecification(instance_id)), &info) == false) {
		LOG(kAppName, liHigh, "Got a protocol message which cannot be matched to an existing account");
		return;
	};		

	const char *status = NULL;
	if (msg->FindString("status", &status) != B_OK) {
		_ERROR("ERROR: No status in UpdateStatus message", msg);
		return;
	};
	
	Connection connection(protocol, info->AccountName(), id);	
	std::string new_status = status;
	
	LOG(kAppName, liMedium, "STATUS_CHANGED [%s] is now %s", connection.String(), new_status.c_str());
	
	// add old status to msg
	if ( fStatus[connection.String()] != "" ) {
		msg->AddString("old_status", fStatus[connection.String()].c_str());
	} else {
		msg->AddString("old_status", OFFLINE_TEXT);
	};
	
	// update status
	fStatus[connection.String()] = new_status;
	
	// Add old total status to msg, to remove duplicated message to user
	entry_ref ref;
	for (int i = 0; msg->FindRef("contact", i, &ref) == B_OK; i++) {
		Contact contact;
		contact.SetTo(&ref);
		
		char total_status[512];
		if (contact.GetStatus(total_status, sizeof(total_status)) == B_OK) {
			msg->AddString("old_total_status", total_status);
		};
		
		// calculate total status for contact
		// Switch this to query for all contacts on all drives that have this connection
		// and update status for all of them.
		GenericListStore<ContactCachedConnections *> contacts = fContact->FindAll(new ConnectionContactSpecification(connection));
		for (GenericListStore<ContactCachedConnections *>::Iterator iter = contacts.Start(); iter != contacts.End(); iter++ ) {
			Contact *contact = (*iter);
			UpdateContactStatusAttribute(*contact);
		};
		
		// Add new total status to msg, to remove duplicated message to user
		if (contact.GetStatus(total_status, sizeof(total_status)) == B_OK) {
			msg->AddString("total_status", total_status);
		};
	};
	
	// Play sound event if needed
	if (new_status != msg->FindString("old_status")) {
		if (new_status == ONLINE_TEXT) {
			system_beep(kImStatusOnlineSound);
		};
		if (new_status == AWAY_TEXT) {
			system_beep(kImStatusAwaySound);
		};
		if (new_status == OFFLINE_TEXT) {
			system_beep(kImStatusOfflineSound);
		};
	};
};

/**
	Calculate total status for contact and update IM:status attribute
	and the icons too.
*/
void Server::UpdateContactStatusAttribute(Contact &contact) {
	// Calculate total status for contact
	std::string new_status = OFFLINE_TEXT;
	
	for (int i = 0; i < contact.CountConnections(); i++) {
		char connection[512];

		contact.ConnectionAt(i,connection);

		std::string curr = fStatus[connection];

		if (curr == ONLINE_TEXT) {
			new_status = ONLINE_TEXT;
			break;
		};

		if ((curr == AWAY_TEXT) && (new_status == OFFLINE_TEXT)) {
			new_status = AWAY_TEXT;
		};
	};

	// Update status attribute
	BNode node(contact);

	if (node.InitCheck() != B_OK) {
		_ERROR("ERROR: Invalid node when setting new status");
	} else {
		// Node exists, write status
		const char * status = new_status.c_str();
		
		// check if blocked
		char old_status[256];
		bool is_blocked = false;
		
		if (contact.GetStatus(old_status, sizeof(old_status)) == B_OK) {
			if (strcmp(old_status, BLOCKED_TEXT) == 0) {
				is_blocked = true;
			};
		};

		if (is_blocked == false) {
			// Only update IM:status if not blocked
			if (strcmp(old_status, status) == 0) {
				// status not changed, done
				node.Unset();
				return;
			};

			contact.SetStatus(status);
		} else {
			// Blocked, don't bother updating icons.
			// We SHOULD, on the other hand, bother with icons SOMEWHERE.
			return;
		};

		int32 iconIndex = kOfflineStatusIcon;

		if (strcmp(status, ONLINE_TEXT) == 0)
			iconIndex = kAvailableStatusIcon;
		else if (strcmp(status, AWAY_TEXT) == 0)
			iconIndex = kAwayStatusIcon;
		else if (strcmp(status, BLOCKED_TEXT) == 0)
			iconIndex = kBlockStatusIcon;
		else if (strcmp(status, OFFLINE_TEXT) == 0)
			iconIndex = kOfflineStatusIcon;

		StatusIcon* icon = fStatusIcons[iconIndex];

#if defined(__HAIKU__)
		// Add vector icon attribute
		if (node.WriteAttr(BEOS_ICON_ATTRIBUTE, B_VECTOR_ICON_TYPE, 0, icon->VectorIcon(), icon->VectorIconSize()) != icon->VectorIconSize()) {
			LOG("im_server", liHigh, "Couldn't write HVIF icon attribute to contact...");
			node.RemoveAttr(BEOS_ICON_ATTRIBUTE);
		};

		// Remove BeOS attributes for raster icons
		node.RemoveAttr(BEOS_MINI_ICON_ATTRIBUTE);
		node.RemoveAttr(BEOS_LARGE_ICON_ATTRIBUTE);
#else
		// Add mini icon attribute
		if (node.WriteAttr(BEOS_MINI_ICON_ATTRIBUTE, B_MINI_ICON_TYPE, 0, icon->MiniIcon(), icon->MiniIconSize()) != icon->MiniIconSize()) {
			LOG("im_server", liHigh, "Couldn't write mini icon attribute to contact...");
			node.RemoveAttr(BEOS_MINI_ICON_ATTRIBUTE);
		};

		// Add large icon attribute
		if (node.WriteAttr(BEOS_LARGE_ICON_ATTRIBUTE, B_LARGE_ICON_TYPE, 0, icon->LargeIcon(), icon->LargeIconSize()) != icon->LargeIconSize()) {
			LOG("im_server", liHigh, "Couldn't write large icon attribute to contact...");
			node.RemoveAttr(BEOS_LARGE_ICON_ATTRIBUTE);
		};
#endif
#if defined(ZETA)
		// SVG icon is a bit special atm
		// Copy the BEOS_SVG_ICON_EXTRA thing is not needed in Zeta > RC3
		BPath prefsPath;

		// Get and set SVG icon
		if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath, true, NULL) == B_OK) {
			prefsPath.Append("im_kit/icons/");

			BString path(prefsPath.Path());

			path.Append("/");
			path.Append(status);

			BNode svgNode(path.String());

			LOG("im_server", liDebug, "SVG icon path: %s", path.String());

			int32 len = 0;

			void* svg_icon = ReadAttribute(svgNode, BEOS_SVG_ICON_ATTRIBUTE, &len);

			if (len > 0) {
				node.RemoveAttr(BEOS_SVG_ICON_ATTRIBUTE); // This is BAD, we shouldn't need this!
				WriteAttribute(node, BEOS_SVG_ICON_ATTRIBUTE, (char*)svg_icon, len, BEOS_SVG_ICON_ATTRIBUTE_TYPE);
				free(svg_icon);
			} else {
				LOG("im_server", liDebug, "Error reading attribute %s", BEOS_SVG_ICON_ATTRIBUTE);
				node.RemoveAttr(BEOS_SVG_ICON_ATTRIBUTE);
			};

			len = 0;

			svg_icon = ReadAttribute(svgNode, BEOS_SVG_EXTRA_ATTRIBUTE, &len);

			if (len > 0) {
				WriteAttribute(node, BEOS_SVG_EXTRA_ATTRIBUTE, (char*)svg_icon, len, BEOS_SVG_EXTRA_ATTRIBUTE_TYPE);
				free(svg_icon);
			} else {
				LOG("im_server", liDebug, "Error reading attribute %s", BEOS_SVG_EXTRA_ATTRIBUTE);
				node.RemoveAttr(BEOS_SVG_EXTRA_ATTRIBUTE);
			};
		};
#endif
	};

	node.Unset();
};


/**
	Query for all files with a IM:status and set it to OFFLINE_TEXT
*/
void Server::SetAllOffline(void) {
	BVolumeRoster vroster;
	BVolume vol;
	char volName[B_FILE_NAME_LENGTH];

	vroster.Rewind();

	BMessage msg;
	entry_ref entry;

	// Query for all contacts on all drives first
	while (vroster.GetNextVolume(&vol) == B_OK) {
		if ((vol.InitCheck() != B_OK) || (vol.KnowsQuery() != true)) {
			continue;
		};

		vol.GetName(volName);
		LOG("im_server", liLow, "SetAllOffline: Getting contacts on %s", volName);

		BQuery query;

		query.SetPredicate( "IM:connections=*" );
		query.SetVolume(&vol);
		query.Fetch();

		while (query.GetNextRef(&entry) == B_OK) {
			msg.AddRef("contact", &entry);
		};
	};

	// Set status of all found contacts to OFFLINE_TEXT (skipping blocked ones)
	char nickname[512], name[512], filename[512], status[512];

	Contact c;
	for (int i = 0; msg.FindRef("contact", i, &entry) == B_OK; i++) {
		c.SetTo(&entry);

		if (c.InitCheck() != B_OK) {
			_ERROR("SetAllOffline: Contact invalid");
		};

		if (c.GetNickname(nickname,sizeof(nickname)) != B_OK) {
			strcpy(nickname, "<no nick>");
		};
		if (c.GetName(name,sizeof(name)) != B_OK) {
			strcpy(name, "<no name>");
		};
		
		BEntry e(&entry);
		if (e.GetName(filename) != B_OK) {
			strcpy(filename, "<no filename?!>");
		};

		if (c.GetStatus(status, sizeof(status)) == B_OK) {
			if (strcmp(status, BLOCKED_TEXT) == 0) {
				LOG("im_server", liDebug, "Skipping blocked contact %s (%s), filename: %s", name, nickname, filename);
				continue;
			};
		};

		LOG("im_server", liDebug, "Setting %s (%s) offline, filename: %s", name, nickname, filename);

		if (c.SetStatus(OFFLINE_TEXT) != B_OK) {
			LOG("im_server", liDebug, "  error.");
		};

		BNode node(&entry);

		int32 iconIndex = kOfflineStatusIcon;
		StatusIcon* icon = fStatusIcons[iconIndex];

#if defined(__HAIKU__)
		// Add vector icon attribute
		if (node.WriteAttr(BEOS_ICON_ATTRIBUTE, B_VECTOR_ICON_TYPE, 0, icon->VectorIcon(), icon->VectorIconSize()) != icon->VectorIconSize()) {
			LOG("im_server", liHigh, "Couldn't write HVIF icon attribute to contact...");
			node.RemoveAttr(BEOS_ICON_ATTRIBUTE);
		};

		// Remove BeOS attributes for raster icons
		node.RemoveAttr(BEOS_MINI_ICON_ATTRIBUTE);
		node.RemoveAttr(BEOS_LARGE_ICON_ATTRIBUTE);
#else
		// Add mini icon attribute
		if (node.WriteAttr(BEOS_MINI_ICON_ATTRIBUTE, B_MINI_ICON_TYPE, 0, icon->MiniIcon(), icon->MiniIconSize()) != icon->MiniIconSize()) {
			LOG("im_server", liHigh, "Couldn't write mini icon attribute to contact...");
			node.RemoveAttr(BEOS_MINI_ICON_ATTRIBUTE);
		};

		// Add large icon attribute
		if (node.WriteAttr(BEOS_LARGE_ICON_ATTRIBUTE, B_LARGE_ICON_TYPE, 0, icon->LargeIcon(), icon->LargeIconSize()) != icon->LargeIconSize()) {
			LOG("im_server", liHigh, "Couldn't write large icon attribute to contact...");
			node.RemoveAttr(BEOS_LARGE_ICON_ATTRIBUTE);
		};
#endif
#if defined(ZETA)
		// SVG icon is a bit special atm
		// Copy the BEOS_SVG_ICON_EXTRA thing is not needed in Zeta > RC3
		BPath prefsPath;
	
		// Get and set SVG icon
		if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath, true, NULL) == B_OK) {
			prefsPath.Append("im_kit/icons/");

			BString path(prefsPath.Path());
			path.Append("/" OFFLINE_TEXT);

			BNode svgNode(path.String());

			LOG("im_server", liDebug, "SVG icon path: %s", path.String());

			int32 len = 0;
			
			void* svg_icon = ReadAttribute(svgNode, BEOS_SVG_ICON_ATTRIBUTE, &len);

			if (len > 0) {
				node.RemoveAttr(BEOS_SVG_ICON_ATTRIBUTE);
				WriteAttribute(node, BEOS_SVG_ICON_ATTRIBUTE, (char*)svg_icon, len, BEOS_SVG_ICON_ATTRIBUTE_TYPE);
				free(svg_icon);
			} else {
				LOG("im_server", liDebug, "Error reading attribute %s", BEOS_SVG_ICON_ATTRIBUTE);
				node.RemoveAttr(BEOS_SVG_ICON_ATTRIBUTE);
			};

/*			len = 0;
			
			svg_icon = ReadAttribute( svgNode, BEOS_SVG_EXTRA_ATTRIBUTE, &len );
			
			if ( len > 0 ) {
				WriteAttribute( node, BEOS_SVG_EXTRA_ATTRIBUTE, (char*)svg_icon, len, BEOS_SVG_EXTRA_ATTRIBUTE_TYPE );
				free( svg_icon );
			} else {
				LOG("im_server", liDebug, "Error reading attribute %s", BEOS_SVG_EXTRA_ATTRIBUTE);
				node.RemoveAttr(BEOS_SVG_EXTRA_ATTRIBUTE);
			}
*/
		};
#endif
		node.Unset();
	};
};

/**
	Add the ID of all contacts that have a connections for this protocol
	to msg.
*/
void Server::GetContactsForProtocol(const char * _protocol, BMessage * msg) {
	BString orig_protocol(_protocol);
	orig_protocol.ToLower();
	
	BString protoUpper(_protocol), protoLower(_protocol);
	protoUpper.ToUpper();
	protoLower.ToLower();
	
	BString regexp;
	for (int i = 0; i < protoUpper.Length(); i++) {
		if (!isalpha((int)(protoUpper[i]))) {
			// "[--]" seems to have a special meaning for the query engine so it
			// would fail if we did the same expansion we were doing as "-"
			// would become "[--]". As it probably happens with other chars too,
			// I am doing this isalpha() thing here. WARNING! Does it work with
			// UTF8 at all?
			regexp << protoUpper[i];
		} else {
			regexp << "[";
			regexp << protoUpper[i];
			regexp << protoLower[i];
			regexp << "]";
		};
	};
	
	const char *protocol = regexp.String();
	
	BVolumeRoster vroster;
	BVolume	vol;
	Contact result;
	BQuery query;
	std::list <entry_ref> refs;
	char volName[B_FILE_NAME_LENGTH];

	vroster.Rewind();

	while (vroster.GetNextVolume(&vol) == B_OK) {
		if ((vol.InitCheck() != B_OK) || (vol.KnowsQuery() != true)) {
			continue;
		};
		
		vol.GetName(volName);
		LOG(kAppName, liLow, "GetContactsForProtocol: Looking for contacts on %s with protocol %s", volName, protocol);
		query.PushAttr("IM:connections");
		query.PushString(protocol);
		query.PushOp(B_CONTAINS);
		
		query.SetVolume(&vol);
		
		query.Fetch();
		
		entry_ref entry;
		
		while (query.GetNextRef(&entry) == B_OK) {
			refs.push_back(entry);
		};
		
		query.Clear();
	};
	
	std::list <entry_ref>::iterator iter;
	for (iter = refs.begin(); iter != refs.end(); iter++) {
		Contact c(*iter);
		char connBuffer[256];
		
		if (c.FindConnection(orig_protocol.String(), connBuffer) == B_OK) {
			Connection conn(connBuffer);
			msg->AddString("id", conn.ID());
		};
	};
	
	refs.clear();
};

/**
	Generate a settings template for im_server settings
*/
BMessage Server::GenerateSettingsTemplate(void) {
	BMessage main_msg(SETTINGS_TEMPLATE);
	
	BMessage blink_db;
	blink_db.AddString("name", "blink_db");
	blink_db.AddString("description", "Blink Deskbar icon");
	blink_db.AddInt32("type", B_BOOL_TYPE );
	blink_db.AddBool("default", true );
	
	main_msg.AddMessage("setting", &blink_db);
	
	BMessage auto_start;
	auto_start.AddString("name", "auto_start");
	auto_start.AddString("description", "Auto-start im_server");
	auto_start.AddInt32("type", B_BOOL_TYPE );
	auto_start.AddBool("default", true );
	
	main_msg.AddMessage("setting", &auto_start);
	
	BMessage deskbar_icon;
	deskbar_icon.AddString("name", "deskbar_icon");
	deskbar_icon.AddString("description", "Show icon in Deskbar");
	deskbar_icon.AddInt32("type", B_BOOL_TYPE );
	deskbar_icon.AddBool("default", true );
	
	main_msg.AddMessage("setting", &deskbar_icon);
	
	BMessage log_level;
	log_level.AddString("name", "log_level");
	log_level.AddString("description", "Debug log threshold");
	log_level.AddInt32("type", B_STRING_TYPE );
	log_level.AddString("valid_value", "Debug");
	log_level.AddString("valid_value", "Low");
	log_level.AddString("valid_value", "Medium");
	log_level.AddString("valid_value", "High");
	log_level.AddString("valid_value", "Quiet");
	log_level.AddString("default", "Medium");
	
	main_msg.AddMessage("setting", &log_level);
	
	BMessage default_away;
	default_away.AddString("name", "default_away");
	default_away.AddString("description", "Away Message");
	default_away.AddInt32("type", B_STRING_TYPE);
	default_away.AddString("default", "I'm not here right now");
	default_away.AddBool("multi_line", true);

	main_msg.AddMessage("setting", &default_away);
	
	BMessage personHandler;
	personHandler.AddString("name", "person_handler");
	personHandler.AddString("description", "Person handler");
	personHandler.AddInt32("type", B_STRING_TYPE);
	personHandler.AddString("default", "application/x-vnd.m_eiman.sample_im_client");
	
	main_msg.AddMessage("setting", &personHandler);
	
	BMessage appsig;
	appsig.AddString("name", "app_sig");
	appsig.AddString("description", "Application signature");
	appsig.AddInt32("type", B_STRING_TYPE);
	appsig.AddBool("default", IM_SERVER_SIG );
	
	main_msg.AddMessage("setting", &appsig );
	
	return main_msg;
}

/**
	Update im_server settings from message
*/
status_t Server::UpdateOwnSettings(BMessage &settings) {
	LOG(kAppName, liDebug, "Server::UpdateOwnSettings");

	bool auto_start = false;
	bool deskbar_icon = true;

	if (settings.FindBool("auto_start", &auto_start) != B_OK) {
		auto_start = false;
	};

	if (settings.FindBool("deskbar_icon", &deskbar_icon) != B_OK) {
		deskbar_icon = true;
	};

	if (SetAutoStart(auto_start) != B_OK) {
		return B_ERROR;
	};

	if (SetDeskbarIcon(deskbar_icon) != B_OK) {
		return B_ERROR;
	};

	return B_OK;
};

/**
	Apply auto_start setting, by configuring UserBootscript.
*/
status_t Server::SetAutoStart(bool autostart) {
	app_info info;
	BPath serverPath;
	status_t err = B_ERROR;

	// Find server path
	be_roster->GetRunningAppInfo(be_app->Team(), &info);
	serverPath.SetTo(&info.ref);

	// UserBootscript command
	BString cmd0 = "# Added by IM Kit.      AAA_im_server_BBB";
	BString cmd1(serverPath.Path());
	cmd1.Append(" & # AAA_im_server_BBB");

	// Start server at boot time
	BPath path;

	if (find_directory(B_USER_BOOT_DIRECTORY, &path, true) != B_OK) {
		LOG(kAppName, liHigh, "Couldn't find or create B_USER_BOOT_DIRECTORY");
		return B_ERROR;
	};

	// Create file if it doesn't exist
	BDirectory directory(path.Path());
	path.Append("UserBootscript");

	BFile file;
	err = directory.CreateFile(path.Path(), &file, true);
	switch (err) {
		case B_OK: {
		} break;
		case B_FILE_EXISTS: {
			file.SetTo(path.Path(), B_READ_WRITE);
		} break;
		default: {
			LOG(kAppName, liHigh, "Couldn't find or create UserBootscript");
			return B_ERROR;
		} break;
	};

	// Look if we already have autostart here
	ssize_t bytesRead = -1;
	char ch;
	BString buffer;
	std::vector<BString> data;
	bool done = false;
	do {
		bytesRead = file.Read(&ch, 1);
		if (bytesRead < 0) {
			break;
		};

		if (ch == '\n') {
			if (buffer == cmd0) {
				buffer = "";
				continue;
			} else if (buffer == cmd1) {
				done = true;
				break;
			} else {
				data.push_back(buffer);
				buffer = "";
			};
		} else {
			buffer << ch;
		};
	} while (bytesRead > 0);

	if ((autostart == true) && (done == false)) {
		file.Seek(0, SEEK_END);
		file.Write(cmd0.String(), cmd0.Length());
		file.Write("\n", 1);
		file.Write(cmd1.String(), cmd1.Length());
		file.Write("\n", 1);
	} else if ((autostart == false) && (done == true)) {
		std::vector<BString>::iterator it;

		// Close the file and recrete it
		file.Unset();
		file.SetTo(path.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);

		// Now write all the lines except the autostart lines
		for (it = data.begin(); it != data.end(); ++it) {
			buffer = (*it);
			file.Write(buffer.String(), buffer.Length());
			file.Write("\n", 1);
		};
	};

	return B_OK;
};

/**
	Checks if Deskbar is running and then installs deskbar icon.
*/
status_t Server::SetDeskbarIcon(bool deskbar_icon) {
	BDeskbar deskbar;
	bool isDeskbarRunning = true;
	bool isInstalled = false;

	// If the Deskbar is not alive, acknowledge this request to wake it up
#ifdef __HAIKU__
	isDeskbarRunning = deskbar.IsRunning();
#endif
	isInstalled = deskbar.HasItem(DESKBAR_ICON_NAME);

#ifdef __HAIKU__
	// Wait up to 10 seconds for Deskbar to become available, in case it's not running yet
	int32 tries = 10;
	while (!isDeskbarRunning && --tries) {
		if (deskbar.IsRunning()) {
			isDeskbarRunning = true;
			break;
		};
		snooze(1000000);
	};
#endif

	if (isDeskbarRunning == false) {
		LOG(kAppName, liHigh, "Deskbar is not running, giving up...");
		return B_ERROR;
	};

	// Remove the icon
	if (deskbar_icon == false) {
		return deskbar.RemoveItem(DESKBAR_ICON_NAME);
	};

	// Install the icon
	_InstallDeskbarIcon();

	return B_OK;
};

/**
	Start all registered auto-start apps
*/
void Server::StartAutostartApps(void) {
	BMessage clients;
	BMessage clientMsg;

	im_get_client_list(&clients);

	for (int32 i = 0; clients.FindMessage("client", i, &clientMsg) == B_OK; i++) {
		const char *client = clientMsg.FindString("file");

		if (strcmp("im_server", client) == 0) {
			continue;
		};

		BMessage settings;

		if (im_load_client_settings(client, &settings) == B_OK) {
			bool auto_start = false;
			const char *app_sig = NULL;

			settings.FindBool("auto_start", &auto_start);
			app_sig = settings.FindString("app_sig");

			if ((auto_start == true) && (app_sig != NULL)) {
				LOG(kAppName, liLow, "Starting app [%s]", app_sig);
				be_roster->Launch(app_sig);
			};
		};
	};
};

/**
	Stop all registered auto-start apps
*/
void Server::StopAutostartApps(void) {
	BMessage clients;
	BMessage clientMsg;

	im_get_client_list(&clients);

	for (int32 i = 0; clients.FindMessage("client", i, &clientMsg) == B_OK; i++) {
		const char *client = clientMsg.FindString("file");

		if (strcmp("im_server", client) == 0) {
			continue;
		};

		BMessage settings;

		if (im_load_client_settings(client, &settings) == B_OK) {
			bool auto_start = false;
			const char *app_sig = NULL;
			
			settings.FindBool("auto_start", &auto_start);
			app_sig = settings.FindString("app_sig");
			
			if ((auto_start == true) && (app_sig != NULL)) {
				LOG(kAppName, liLow, "Stopping app [%s]", app_sig);
				
				BMessenger msgr(app_sig);
				msgr.SendMessage(B_QUIT_REQUESTED);
			};
		};
	};
};

/**
	Forward a message to deskbar icon
*/
void Server::handleDeskbarMessage(BMessage * msg) {
	switch (msg->what) {
		case REGISTER_DESKBAR_MESSENGER: {
			LOG(kAppName, liDebug, "Got Deskbar messenger");
			msg->FindMessenger("msgr", &fDeskbarMsgr);
		} break;
		
		default: {
			LOG(kAppName, liDebug, "Forwarding message to Deskbar");
			if (fDeskbarMsgr.SendMessage(msg) != B_OK) {
				LOG(kAppName, liMedium, "Error sending message to Deskbar");
			};
		} break;
	};
};

/**
	Handle a STATUS_SET message, update per-protocol and total status
*/
void Server::handle_STATUS_SET(BMessage * msg) {
	const char *protocol = msg->FindString("protocol");
	const char *status = msg->FindString("status");
		
	if (status == NULL) {
		_ERROR("ERROR: STATUS_SET: status not in message",msg);
		return;
	};
	
	if (strcmp(ONLINE_TEXT, status) == 0) {
		// we're online. register contacts. (should be: only do this if we were offline)
		LOG(kAppName, liMedium, "Status changed for %s to %s", protocol, status );

		ProtocolInfo *info = NULL;
		if (fProtocol->FindFirst(new SignatureProtocolSpecification(protocol), &info) == false) {
			_ERROR("ERROR: STATUS_SET: Protocol not loaded",msg);
			return;
		};
			
		// THIS IS NOT CORRECT! We need to do this on a "connected" message, not on
		// status changed!
		system_beep( kImConnectedSound );
		
		BMessage connections(MESSAGE);
		connections.AddInt32("im_what", REGISTER_CONTACTS);
//		XXX
		GetContactsForProtocol(info->Signature(), &connections);

		info->Process(&connections);
	};
	
	if (strcmp(OFFLINE_TEXT,status) == 0) {
		// we're offline. set all connections for protocol to offline
		if (ProtocolOffline(protocol) != B_OK) {
			_ERROR("ERROR: STATUS_SET: Protocol not loaded",msg);
			return;
		};
	};

	// Find out 'total' online status
	fStatus[protocol] = status;
	
	std::string total_status = TotalStatus();

	msg->AddString("total_status", total_status.c_str() );
	LOG(kAppName, liMedium, "Total status changed to %s", total_status.c_str() );
	// end 'Find out total status'
	
	handleDeskbarMessage(msg);
};

/**
	Get current online status per-protocol for a contact
*/
void Server::reply_GET_CONTACT_STATUS(BMessage * msg) {
	entry_ref ref;
	
	if (msg->FindRef("contact",&ref) != B_OK) {
		_ERROR("Missing contact in GET_CONTACT_STATUS", msg);
		return;
	};
	
	Contact contact(ref);
	char connection[255];
	
	BMessage reply;
	reply.AddRef("contact", &ref);
	
	for (int i = 0; contact.ConnectionAt(i, connection) == B_OK; i++) {
		reply.AddString("connection", connection);
		if (fStatus[connection] == "") {
			reply.AddString("status", OFFLINE_TEXT );
		} else {
			reply.AddString("status", fStatus[connection].c_str() );
		};
	};
	
	sendReply(msg,&reply);
};

/**
	Get list of current own online status per protocol
*/
void Server::reply_GET_OWN_STATUSES(BMessage *msg) {
	LOG(kAppName, liLow, "Got own status request. There are %i statuses", fStatus.size());

	BMessage reply(B_REPLY);

	GenericListStore<ProtocolInfo *> protocols = fProtocol->FindAll(
		new NotSpecification<ProtocolInfo *>(
			new ExitedProtocolSpecification()
		)
	);

	for (GenericListStore<ProtocolInfo *>::Iterator pIt = protocols.Start(); pIt != protocols.End(); pIt++) {
		ProtocolInfo *info = (*pIt);

		reply.AddString("protocol", info->Signature());
		reply.AddString("status", fStatus[info->Signature()].c_str());
		reply.AddString("account_name", info->AccountName());
		reply.AddString("instance_id", info->InstanceID());
		reply.AddString("userfriendly", info->FriendlySignature());
	};

	sendReply(msg,&reply);
};

/**
	Returns a list of currently loaded protocols
*/
void Server::reply_GET_LOADED_PROTOCOLS(BMessage * msg) {
	BMessage reply(ACTION_PERFORMED);
	GenericListStore<ProtocolInfo *> protocols = fProtocol->FindAll(new AllProtocolSpecification());
	GenericListStore<ProtocolInfo *>::Iterator it;
	
	for (it = protocols.Start(); it != protocols.End(); it++) {
		ProtocolInfo *info = (*it);
		entry_ref ref;
		
		if ((info->HasValidMessenger() == true) && (get_ref_for_path(info->Path().Path(), &ref) == B_OK)) {
			reply.AddString("protocol", info->Signature());
			reply.AddRef("ref", &ref);
		} else {
			LOG(kAppName, liLow, "reply_GET_LOADED_PROTOCOLS: Found a half-loaded protocol: %s", info->Path().Path());
		};
	};

	sendReply(msg,&reply);
};

/**
	?
*/
void Server::reply_SERVER_BASED_CONTACT_LIST(BMessage * msg) {
	const char *id = NULL;

	const char *protocol = NULL;
	if (msg->FindString("protocol", &protocol) != B_OK) {
		_ERROR("ERROR: Malformed SERVER_BASED_CONTACT_LIST message", msg);
		return;
	};
	
	const char *instance_id = NULL;
	if (msg->FindString("instance_id", &instance_id) != B_OK) {
		LOG(kAppName, liHigh, "Got a protocol message with no instance!");
		return;
	};
	
	ProtocolInfo *info = NULL;
	if (fProtocol->FindFirst(new AndProtocolSpecification(new SignatureProtocolSpecification(protocol), new InstanceProtocolSpecification(instance_id)), &info) == false) {
		LOG(kAppName, liHigh, "Got a protocol message which cannot be matched to an existing account");
		return;
	};		
	// For each ID
	for (int i = 0; msg->FindString("id", i, &id) == B_OK; i++) {
		Connection connection(protocol, info->AccountName(), id);
		ContactCachedConnections *c = NULL;

		if ((fContact->FindFirst(new ConnectionContactSpecification(connection), &c) == false) || (c->InitCheck() != B_OK)) {
			fContact->CreateContact(connection, id);
		};
	};
};

/**
*/
void Server::reply_UPDATE_CONTACT_STATUS(BMessage * msg) {
	entry_ref ref;
	
	if (msg->FindRef("contact", &ref) != B_OK) {
		_ERROR("Missing contact in UPDATE_CONTACT_STATUS",msg);
		return;
	};
	
	Contact contact(ref);
	
	UpdateContactStatusAttribute(contact);
};

/*
*/

void Server::reply_GET_ALL_CONTACTS(BMessage *msg) {
	BMessage reply(B_REPLY);

	GenericListStore<ContactCachedConnections *> contacts = fContact->FindAll(new AllContactSpecification());
	
	for (GenericListStore<ContactCachedConnections *>::Iterator it = contacts.Start(); it != contacts.End(); it++) {
		Contact *contact = (*it);
		
		reply.AddRef("contact", *contact);
	};

	sendReply(msg,&reply);
};

/**
*/
void Server::handle_SETTINGS_UPDATED(BMessage *msg) {
	BMessage settings;
	const char *sig = NULL;
	
	if (msg->FindString("protocol", &sig) == B_OK) {
		// Case 1: Currently launched protocol - update settings
		// Case 2: New settings - launch protocol
		// Case 3: Removed protocol - Shutdown launcher
		// A renamed should show up as a new and a remove
	
		bool accountsChanged = false;
		BMessage accounts;
		BString account;
		im_protocol_get_account_list(sig, &accounts);

		GenericListStore<ProtocolInfo *> protoAccounts = fProtocol->FindAll(new SignatureProtocolSpecification(sig));
		GenericListStore<BString> curAccounts;	
		
		for (int32 i = 0; accounts.FindString("account", i, &account) == B_OK; i++) {
			curAccounts.Add(account);			
			ProtocolInfo *info = NULL;
			
			if ((protoAccounts.FindFirst(new AccountNameProtocolSpecification(account), &info) == true) && (info != NULL)) {
				// Existing account - update settings
				BMessage settings;
				im_protocol_get_account(sig, account.String(), &settings);
				
				info->UpdateSettings(&settings);
			} else {
				// New account - launch protocol
				entry_ref settingsRef;
				accounts.FindRef("settings_path", i, &settingsRef);				
				
				BPath settingsPath(&settingsRef);
				BPath protocolPath;
				
				if (im_protocol_get_path(sig, &protocolPath) == B_OK) {
					info = fProtocol->LaunchInstance(protocolPath, settingsPath, account.String());
					accountsChanged = true;
					
					LOG(kAppName, liMedium, "%s launched with as \"%s\"", account.String(), info->InstanceID());
				};				
			};
		};
		
		// Now loop over the currently launched Accounts and determine which ones need to be shutdown
		for (GenericListStore<ProtocolInfo *>::Iterator it = protoAccounts.Start(); it != protoAccounts.End(); it++) {
			ProtocolInfo *info = (*it);
			
			GenericListStore<BString> items = curAccounts.FindAll(new EqualStringSpecification(info->AccountName()));
			if (items.CountItems() == 0) {
				// Protocol is no longer active - shut it down
				fProtocol->UnloadInstance(info);
				
				accountsChanged = true;
			};
		};
		
		if (accountsChanged == true) {
			BMessage changed(LOADED_PROTOCOLS_CHANGED);
			Broadcast(&changed);
		};
	} else {
		if (msg->FindString("client", &sig) == B_OK) {
			if (strcmp("im_server", sig) == 0) {
				if (im_load_client_settings(sig, &settings) == B_OK) {
					UpdateOwnSettings(settings);
				};
			};
		
			Broadcast(msg);
		} else {
			// malformed
			_ERROR("Malformed message in SETTINGS_UPDATED", msg);
		};
	};
};

void Server::InitSettings(void) {
	// Save settings template
	BMessage tmplate = GenerateSettingsTemplate();
	
	im_save_client_template("im_server", &tmplate);
	
	// Make sure default settings are there
	BMessage settings;
	bool temp = false;
	const char *str = NULL;;
	im_load_client_settings("im_server", &settings);
	
	if (settings.FindString("app_sig", &str) != B_OK) {
		settings.AddString("app_sig", IM_SERVER_SIG);
	};
		
	if (settings.FindBool("auto_start", &temp) != B_OK) {
		settings.AddBool("auto_start", false);
	};
		
	if (settings.FindString("log_level", &str) != B_OK) {
		settings.AddString("log_level", "High");
	};

	if (settings.FindString("default_away", &str) != B_OK) {
		settings.AddString("default_away", "I'm not here right now");
	};
		
	// Done with template and settings.
	im_save_client_settings("im_server", &settings);
};

void Server::reply_GET_CONTACTS_FOR_PROTOCOL(BMessage * msg) {
	if (msg->FindString("protocol") == NULL) {
		msg->SendReply(ERROR);	
		return;
	};
	
	BMessage reply(ACTION_PERFORMED);

//	XXX	
	GetContactsForProtocol(msg->FindString("protocol"), &reply);

	sendReply(msg,&reply);
};

void Server::ReadyToRun(void) {
	fContact->Init();

	SetAllOffline();
	StartAutostartApps();
};

void Server::sendReply(BMessage *msg, BMessage *reply) {
	// add friendly protocol name if applicable
	const char *protocol = NULL;
	
	for (int i = 0; reply->FindString("protocol", i, &protocol) == B_OK; i++) {
		const char *userfriendly = "<invalid protocol>";
		
		ProtocolInfo *info = NULL;
		if (fProtocol->FindFirst(new SignatureProtocolSpecification(protocol), &info) == true) {
			userfriendly = info->FriendlySignature();
		};
		
		reply->AddString("userfriendly",  userfriendly);
	};
	// done adding fancy names
	
	msg->SendReply(reply);
};

const char *Server::TotalStatus(void) {
	std::string total_status = OFFLINE_TEXT;
	
	GenericListStore<ProtocolInfo *> protocols = fProtocol->FindAll(new AllProtocolSpecification());
	for (GenericListStore<ProtocolInfo *>::Iterator i = protocols.Start(); i != protocols.End(); i++) {
		ProtocolInfo *info = (*i);
	
		if (fStatus[info->Signature()] == ONLINE_TEXT) {
			total_status = ONLINE_TEXT;
			break;
		};
		
		if (fStatus[info->Signature()] == AWAY_TEXT) {
			total_status = AWAY_TEXT;
		};
	};

	return total_status.c_str();	
};

status_t Server::ProtocolOffline(const char *signature) {
	ProtocolInfo *info = NULL;
	if (fProtocol->FindFirst(new SignatureProtocolSpecification(signature), &info) == false) {
		LOG(kAppName, liHigh, "Unexpected protocol went offline: %s", signature);
		return B_ERROR;
	};
		
	system_beep(kImDisconnectedSound);
	
	
	BMessage contacts;
	// XXX
	GetContactsForProtocol(signature, &contacts);
	const char *id = NULL;
	
	for (int i = 0; contacts.FindString("id", i, &id) == B_OK; i++) {
		// XXX: TODO: Needs to use Connection and include an Account
		std::string proto_id;
		proto_id += signature;
		proto_id += ":";
		proto_id += id;
		
		LOG(kAppName, liDebug, "Protocol offline, setting %s offline", proto_id.c_str());
		
		if ((fStatus[proto_id] != OFFLINE_TEXT) && (fStatus[proto_id] != "")) {
			// only send a message if there's been a change.
			BMessage update(MESSAGE);
			update.AddInt32("im_what", STATUS_CHANGED);
			update.AddString("protocol", signature);
			update.AddString("id", contacts.FindString("id",i));
			update.AddString("status", OFFLINE_TEXT);
			
			BMessenger(this).SendMessage(&update);
			
			fStatus[proto_id] = OFFLINE_TEXT;
			
			// update status attribute
			GenericListStore<ContactCachedConnections *> contacts = fContact->FindAll(new ConnectionContactSpecification(proto_id.c_str()));
		
			for (GenericListStore<ContactCachedConnections *>::Iterator it = contacts.Start(); it != contacts.End(); it++) {
				Contact *contact = (*it);
				UpdateContactStatusAttribute(*contact);
			};
		} else {
			LOG(kAppName, liDebug, "Already offline, or no previous status" );
		};
	};
		
	fStatus[signature] = OFFLINE_TEXT;
		
	return B_OK;
};


void Server::ChildExited(int /*sig */, void *data, struct vreg */*regs */) {
	Server *server = (Server *)data;

	if (server->fIsQuitting == false) {
		
		ProtocolSpecification *spec = new AndSpecification<ProtocolInfo *>(
			new ExitedProtocolSpecification(),
			new AllowRestartProtocolSpecification()
		);
		GenericListStore<ProtocolInfo *> protocols = server->fProtocol->FindAll(spec, false);
	
		for (GenericListStore<ProtocolInfo *>::Iterator pIt = protocols.Start(); pIt != protocols.End(); pIt++) {
			ProtocolInfo *info = (*pIt);
	
			if (server->ProtocolOffline(info->Signature()) == B_OK) {
				// Tell everyone the protocol has gone offline
				BMessage offline(MESSAGE);
				offline.AddInt32("im_what", STATUS_SET);
				offline.AddString("protocol", info->Signature());
				offline.AddString("status", OFFLINE_TEXT);
				offline.AddString("total_status", server->TotalStatus());
					
				server->handleDeskbarMessage(&offline);
	
				// Broadcast the message
				BMessage changed(LOADED_PROTOCOLS_CHANGED);		
				server->Broadcast(&changed);
			};
		};
		
		server->fProtocol->RestartProtocols(spec, false);
				
		delete spec;
	};
};
