#ifndef JABBER_HANDLER_H
#define JABBER_HANDLER_H

#include <String.h>
#include <list>
using std::list;

#include "JabberAgent.h"
#include "JabberContact.h"
#include "JabberElement.h"
#include "JabberMessage.h"
#include "JabberPresence.h"
#include "JabberRegistration.h"
#include "JabberPlug.h"

#include <common/ObjectList.h>

#include <libexpat/xmltok.h>
#include <libexpat/expat.h>
#include <libexpat/xmlrole.h>

class JabberHandler 
{
public:
								JabberHandler(const BString & name, JabberPlug*);
	virtual 					~JabberHandler();
	void						Dispose();			
	void 						LogOn();
	void 						LogOff();
	
	//void 						Register();
	void 						Register(JabberRegistration * registration);
	void 						Register(JabberAgent * agent);
	
	BString 					GetName() const;
	
	void 						SetHost(const BString & host);
	void 						SetUsername(const BString & username);
	void 						SetPassword(const BString & password);
	void 						SetPort(int32 port);
	void 						SetPriority(int32 priority);
	void 						SetResource(const BString & resource);
	
	void 						UpdateJid();
	BString 					GetJid() const;
	
	void 						SetAuthorized(bool authorized);
	bool						IsAuthorized();
	void 						RemoveContact(const JabberContact * contact);
	
	int32 						ReceivedData(const char *,int32);
	
protected: 
	typedef BObjectList<JabberElement> ElementList;
	typedef list<BString>		StrList;
	typedef BObjectList<JabberContact> RosterList;
	typedef BObjectList<JabberAgent> AgentList;

	bool 						SendMessage(JabberMessage & message);
	bool 						StartComposingMessage(JabberContact * contact);
	bool 						StopComposingMessage(JabberContact * contact);
	
	void 						SetStatus(int32 status, const BString & message);
	
	void 						TimeStamp(JabberMessage & message);
	
	void 						AddContact(const BString & name, const BString & jid, const BString & group);
	void 						AcceptSubscription(const BString & jid);
	void 						UpdateRoster(JabberPresence * presence);
	
	
	
	
	// The JabberHandler takes ownership of the contact
	void 						UpdateRoster(JabberContact * contact);
	// by xeD
	RosterList*					getRosterList(){ return fRoster; };
	
	//Callbacks
	virtual void 				Authorized();
	virtual void 				Message(JabberMessage * message)=0; 
	virtual void 				Presence(JabberPresence * presence) =0;
	virtual void 				Roster(RosterList * roster) =0;
	virtual void 				Agents(AgentList * agents) =0;
	virtual void 				Disconnected(const BString & reason) = 0;
	virtual void 				SubscriptionRequest(JabberPresence * presence) = 0;
	virtual void 				Registration(JabberRegistration * registration) = 0;
	virtual	void				Unsubscribe(JabberPresence * presence) = 0;
	
private:
	enum ID 
	{ 
		AUTH = 0, 
		ROSTER = 2
	};
	
	
	BString 					fHost;
	BString 					fUsername;
	BString 					fJid;
	BString 					fPassword;
	int32 						fPort;
	BString 					fResource;
	int32 						fPriority;

	ElementList *				fElementStack;
	StrList *					fNsStack;
	RosterList *				fRoster;
	AgentList *					fAgents;

	int32 						fSocket;
	JabberPlug*					fPlug;
	
	XML_Parser 					fParser;
	bool 						fAuthorized;
	
protected:

	void 						Send(const BString & xml);
	void 						Authorize();
	bool 						BeginSession();
	void 						EndSession();
	void 						SendVersion();
	void 						RequestRoster();
	void 						RequestAgents();
	
	
	
		
	JabberMessage* 				BuildMessage();
	JabberPresence* 				BuildPresence();
	RosterList *				BuildRoster();
	AgentList * 				BuildAgents();
	JabberRegistration * 		BuildRegistration();
			
	//int32 						GetConnection();
	const JabberAgent * 		IsAgent(const BString & jid);
	const char * 				HasAttribute(const char * name, const char ** attributes,int32 count);
	const char * 				HasAttribute(const char * name, const char ** attributes);

	void 						StripResource(BString & jid);
	BString 					TwoDigit(int32, BString &);
	
	static void StartElement(void * pUserData, const char * pName, const char ** pAttr);
	static void EndElement(void * pUserData, const char * pName);
	static void Characters(void * pUserData, const char * pString, int pLen);
};

#endif
