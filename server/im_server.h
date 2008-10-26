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
#include "AddOnInfo.h"

/**
	Used by Contact monitor.
*/
class ContactHandle
{
	public:
		ino_t node;
		entry_ref entry;
		
		ContactHandle()
		{
		}
		
		ContactHandle( const ContactHandle & c )
		:	node( c.node )
		{
			entry.directory = c.entry.directory;
			entry.device = c.entry.device;
			entry.set_name( c.entry.name );
		}
		
		bool operator < ( const ContactHandle & c ) const
		{
			if ( entry.device != c.entry.device )
				return entry.device < c.entry.device;
			
			return node < c.node;
		}
		
		bool operator == ( const ContactHandle & c ) const
		{
/*			printf("%Ld, %ld vs %Ld, %ld\n", 
				node, entry.device, 
				c.node, c.entry.device
			);
*/			
			return node == c.node && entry.device == c.entry.device;
		}
};

namespace IM {

class Server : public BApplication
{
	public:
		Server();
		virtual ~Server();
		
		virtual status_t GetSupportedSuites(BMessage *);
		virtual BHandler *ResolveSpecifier(BMessage *msg, int32 index, BMessage *specifier, int32 what, const char *property);
		
		virtual bool QuitRequested();
		
		virtual void MessageReceived( BMessage * );
		
		virtual void ReadyToRun();
		
	private:
		void	StartQuery();
		void	HandleContactUpdate( BMessage * );
		
		Contact			FindContact( const char * proto_id );
		list<Contact>	FindAllContacts( const char * proto_id );
		Contact			CreateContact( const char * proto_id , const char *namebase );
		
		void		RegisterSoundEvents();
		void		CheckIndexes();
		status_t	LoadAddons();
		void		LoadAddonsFromDir( BDirectory* addonsDir, BDirectory* settingsDir );
		void		UnloadAddons();
		
		bool	IsMessageOk( BMessage * );
		void	Process( BMessage * );
		void	Broadcast( BMessage * );
		
		void	AddEndpoint( BMessenger );
		void	RemoveEndpoint( BMessenger );
		
		status_t	GetSettings(const char * protocol, BMessage*);
		status_t	SetSettings(const char * protocol, BMessage*);
		BMessage	GenerateSettingsTemplate();
		status_t	UpdateOwnSettings( BMessage & );
		void		InitSettings();
		
		void	handleDeskbarMessage( BMessage * );
		
		void	MessageToProtocols(BMessage*);
		void	MessageFromProtocols(BMessage*);
		
		void	UpdateStatus(BMessage*);
		void	SetAllOffline();
		void	handle_STATUS_SET( BMessage * );
		void	UpdateContactStatusAttribute( Contact & );
		
		void	GetContactsForProtocol( const char * protocol, BMessage * msg );
		
		void	StartAutostartApps();
		void	StopAutostartApps();
		
		// Adds "userfriendly" protocol strings, then sends reply
		void	sendReply(BMessage* msg, BMessage* reply);
		
		void	reply_GET_LOADED_PROTOCOLS(BMessage*);
		void	reply_SERVER_BASED_CONTACT_LIST(BMessage*);
		void	reply_GET_CONTACT_STATUS( BMessage * );
		void	reply_UPDATE_CONTACT_STATUS( BMessage * );
		void	reply_GET_OWN_STATUSES(BMessage *msg);
		void	reply_GET_CONTACTS_FOR_PROTOCOL( BMessage * );
		void	reply_GET_ALL_CONTACTS(BMessage *);
		
		void	handle_SETTINGS_UPDATED(BMessage *);
		
		status_t	selectConnection( BMessage * msg, Contact & contact );
		
		/**
			Contact monitoring functions
		*/
		void ContactMonitor_Added( ContactHandle );
		void ContactMonitor_Modified( ContactHandle );
		void ContactMonitor_Moved( ContactHandle from, ContactHandle to );
		void ContactMonitor_Removed( ContactHandle );
		
		// Variables
		
		list<BQuery*>				fQueries;
		list<BMessenger>			fMessengers;
		map<string,Protocol*>		fProtocols;
		map<Protocol*,AddOnInfo>	fAddOnInfo;
		bool						fIsQuiting;
		
		/**
		 * Remove when protocols deal with scripting. Currently all is done in the
		 * application.
		 */
		Protocol                   *fCurProtocol;
		
		/**
			entry_ref, list of connections.
			Used to store connections for contacts, so we can notify the protocols
			of any changes.
		*/
		list< pair<ContactHandle, list<string>* > > fContacts;
		
		/*	Used to store both <protocol>:<id> and <protocol> status.
			In other words, both own status per protocol and contact
			status per connection */
		map<string,string>			fStatus;// proto_id_string,status_string
		
		map<Contact,string>			fPreferredConnection;
		
		BMessage					fIcons;
//		Names for SVG icons, only used on Zeta
//#ifdef B_BEOS_VERSION = B_ZETA_VERSION
//#define SVG_ONLINE_TEXT "svg_online"
//#define SVG_AWAY_TEXT "svg_away"
//#define SVG_OFFLINE_TEXT "svg_offline"
//#endif
		
		BMessenger					fDeskbarMsgr;
};

};

#define LOG_PREFIX "im_server"

#endif
