#ifndef IM_SERVER_H
#define IM_SERVER_H

#include <list>
#include <map>
#include <string>

#include <Application.h>
#include <Messenger.h>
#include <Query.h>
#include <Entry.h>
#include <Node.h>
#include <String.h>

#include <common/IMKitUtilities.h>

#include <libim/Contact.h>
#include <libim/Protocol.h>

#include "ContactHandle.h"

namespace IM {

class ProtocolInfo;
class ProtocolManager;
class StatusIcon;

class Server : public BApplication {
	public:
								Server(void);
		virtual					~Server(void);
		
		// Scripting Hooks
		virtual status_t GetSupportedSuites(BMessage *);
		virtual BHandler *ResolveSpecifier(BMessage *msg, int32 index, BMessage *specifier, int32 what, const char *property);

		// BApplication Hooks		
		virtual bool 			QuitRequested(void);
		virtual void			MessageReceived(BMessage *msg);
		virtual void			ReadyToRun(void);
		
	private:
		void					_Init(void);
		void					_UpdateStatusIcons(void);
		void					_InstallDeskbarIcon(void);
		void					StartQuery(void);
		void					HandleContactUpdate(BMessage *);
		
		Contact					FindContact(const char * proto_id);
		list<Contact>			FindAllContacts(const char * proto_id);
		Contact					CreateContact(const char * proto_id, const char *namebase);
		
		void					RegisterSoundEvents(void);
		void					CheckIndexes(void);
		status_t				LoadProtocols(void);
		
		bool					IsMessageOk(BMessage *);
		void					Process(BMessage *);
		void					Broadcast(BMessage *);
		
		void					AddEndpoint(BMessenger);
		void					RemoveEndpoint(BMessenger);
		
		BMessage				GenerateSettingsTemplate(void);
		status_t				UpdateOwnSettings(BMessage &);
		void					InitSettings(void);
		
		void					handleDeskbarMessage(BMessage *);
		
		void					MessageToProtocols(BMessage*);
		void					MessageFromProtocols(BMessage*);

		void					UpdateStatus(BMessage*);
		void					SetAllOffline(void);
		void					handle_STATUS_SET(BMessage *);
		void					UpdateContactStatusAttribute(Contact &);
		
		void					GetContactsForProtocol(const char * protocol, BMessage * msg);
		
		void					StartAutostartApps(void);
		void					StopAutostartApps(void);
		
		// Adds "userfriendly" protocol strings, then sends reply
		void					sendReply(BMessage* msg, BMessage* reply);
		
		void					reply_GET_LOADED_PROTOCOLS(BMessage*);
		void					reply_SERVER_BASED_CONTACT_LIST(BMessage*);
		void					reply_GET_CONTACT_STATUS( BMessage * );
		void					reply_UPDATE_CONTACT_STATUS( BMessage * );
		void					reply_GET_OWN_STATUSES(BMessage *msg);
		void					reply_GET_CONTACTS_FOR_PROTOCOL( BMessage * );
		void					reply_GET_ALL_CONTACTS(BMessage *);
		
		void					handle_SETTINGS_UPDATED(BMessage *);
		
		status_t				selectConnection(BMessage * msg, Contact & contact);
		
		const char				*TotalStatus(void);
		status_t				ProtocolOffline(const char *signature);
		static void				ChildExited(int signal, void *data, struct vreg *regs);
		
		/**
			Contact monitoring functions
		*/
		void					ContactMonitor_Added(ContactHandle);
		void					ContactMonitor_Modified(ContactHandle);
		void					ContactMonitor_Moved(ContactHandle from, ContactHandle to);
		void					ContactMonitor_Removed(ContactHandle);
		
		// Variables
		
		list<BQuery*>			fQueries;
		list<BMessenger>		fMessengers;
		bool					fIsQuitting;
		
		/**
		 * Remove when protocols deal with scripting. Currently all is done in the
		 * application.
		 */
		ProtocolInfo			*fCurProtocol;
		
		/**
			entry_ref, list of connections.
			Used to store connections for contacts, so we can notify the protocols
			of any changes.
		*/
		list< pair<ContactHandle, list<string>* > > fContacts;
		
		/*	Used to store both <protocol>:<id> and <protocol> status.
			In other words, both own status per protocol and contact
			status per connection */
		map<string,string>		fStatus;// proto_id_string,status_string
		
		map<Contact,string>		fPreferredConnection;

		map<int, StatusIcon *>	fStatusIcons;

		BMessenger				fDeskbarMsgr;
		
		ProtocolManager			*fProtocol;
};

};

#define LOG_PREFIX "im_server"

#endif
