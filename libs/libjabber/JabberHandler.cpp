#include "JabberHandler.h"
#include "SHA1.h"
#include "States.h"
#include "Logger.h"



#include <unistd.h>
#include <List.h>
#include <ListItem.h>
#include <String.h>



JabberHandler::JabberHandler(const BString & name, JabberPlug* plug)  
{
	fHost = fUsername = fJid = fPassword = fResource = "";
	fPort = fPriority = 0;
	
	fElementStack = new ElementList(20, true);
	fNsStack = new StrList;
	fRoster = new RosterList(20, true);
	fAgents = new AgentList(20, true);
	
	fSocket = -1;
	fAuthorized = false;
	
	fPlug = plug;
	
	fParser = XML_ParserCreate(NULL);
	XML_SetUserData(fParser, this);
	XML_SetElementHandler(fParser, StartElement, EndElement);
	XML_SetCharacterDataHandler(fParser, Characters);


}

void
JabberHandler::Dispose() 
{
	
	if(fParser) 			XML_ParserFree(fParser);
	if(fElementStack)	delete fElementStack;
	if(fNsStack) 		delete fNsStack;
	if(fRoster)			delete fRoster;
	if(fAgents)			delete fAgents;
	
	fAgents=NULL;
	fRoster=NULL;
	fNsStack=NULL;
	fElementStack=NULL;
	fParser=NULL;
	

	
}

JabberHandler::~JabberHandler() 
{
	Dispose();
}

void
JabberHandler::UpdateJid()
{
	fJid = "";
	fJid << fUsername << "@" << fHost;
}

BString
JabberHandler::GetJid() const
{
	return fJid;
}

void
JabberHandler::SetHost(const BString & host) 
{
	fHost = host;
	UpdateJid();
}

void
JabberHandler::SetUsername(const BString & username) 
{
	fUsername = username;
	UpdateJid();
}

void
JabberHandler::SetPassword(const BString & password) 
{
	fPassword = password;
}

void
JabberHandler::SetPort(int32 port) 
{
	fPort = port;
}

void
JabberHandler::SetPriority(int32 priority)
{
	fPriority = priority;
}

void
JabberHandler::SetResource(const BString & resource)
{
	fResource = resource;
}

BString
JabberHandler::GetName() const
{
	return fUsername;
}

bool
JabberHandler::SendMessage(JabberMessage & message) 
{
	BString xml;
	BString body = message.GetBody();
	body.ReplaceAll("&", "&amp;");
	body.ReplaceAll("<", "&lt;");
	body.ReplaceAll(">", "&gt;");
	body.ReplaceAll("\'", "&apos;");
	body.ReplaceAll("\"", "&quot;");
	//original by olmeki team
	//xml << "<message type='chat' to='" << message.GetTo() << "'><body>" << body << "</body></message>";
	//modified by xed (we ask for composing events)
	xml << "<message type='chat' to='" << message.GetTo() << "' id='" << message.GetID() << "'><body>" << body << "</body>";
	xml << "<x xmlns='jabber:x:event'><composing/></x>";
	xml << "</message>";
	
	
	Send(xml);
	return true;
}
bool
JabberHandler::StartComposingMessage(JabberContact * contact) 
{
	//send a composing event
	if(contact->GetLastMessageID().ICompare("")==0) return false; //?? no need to send notification
	BString xml;
	
	xml << "<message to='" << contact->GetJid() << "'>";
	if(contact->GetLastMessageID()!="")
	xml << "<x xmlns=\"jabber:x:event\"><composing/><id>" << contact->GetLastMessageID() << "</id></x>";
	xml << "</message>";
		
	Send(xml);
	return true;
}
bool
JabberHandler::StopComposingMessage(JabberContact * contact) 
{
	//send a composing event
	BString xml;
	xml << "<message to='" << contact->GetJid() << "'>";
	xml << "<x xmlns=\"jabber:x:event\"><id></id></x>";
	xml << "</message>";
		
	Send(xml);
	return true;
}

void
JabberHandler::AddContact(const BString & name, const BString & jid, const BString & group) 
{
	BString xml;
	xml << "<iq type='set'><query xmlns='jabber:iq:roster'>";
	xml << "<item name='" << name << "' jid='" << jid << "'>";
	xml << "<group>" << group << "</group></item></query></iq>";
	xml << "<presence to='" << jid << "' type='subscribe'>";
	xml << "<status>I would like to add you to my roster.</status></presence>";
	Send(xml);
}

void
JabberHandler::RemoveContact(const JabberContact * contact)
{
	BString xml;
	xml << "<iq type='set'><query xmlns='jabber:iq:roster'>";
    xml << "<item jid='" << contact->GetJid() << "' subscription='remove'> ";
	xml << "</item></query></iq>";
	Send(xml);
}

void
JabberHandler::AcceptSubscription(const BString & jid) 
{
	BString xml;
	xml << "<presence to='" << jid << "' type='subscribed'/>";
	Send(xml);
}

void
JabberHandler::SetStatus(int32 status, const BString & message) 
{
	BString xml;
	xml << "<presence";
	switch (status) 
	{
		case S_ONLINE:
			xml << ">";
			break;
		case S_OFFLINE:
			xml << " type='unavailable'>";
			break;
		case S_AWAY:
			xml << "><show>away</show>";
			break;
		case S_XA:
			xml << "><show>xa</show>";
			break;
		case S_DND:
			xml << "><show>dnd</show>";
			break;
		case S_CHAT:
			xml << "><show>chat</show>";
			break;
		default:
			return;
	}
	if (message != "")
		xml << "<status>" << message << "</status>";
	
	// Aha, fix the priority now as well :)
	xml << "<priority>" << fPriority << "</priority></presence>";

	if (!fAuthorized) 
		LogOn();
		
	Send(xml);

	if (status == S_OFFLINE)
		LogOff();
}

void
JabberHandler::LogOff() 
{
	EndSession();
	fAuthorized = false;
	Disconnected("");
}

void
JabberHandler::SetAuthorized(bool authorized) 
{
	fAuthorized = authorized;
}
bool
JabberHandler::IsAuthorized() 
{
	return fAuthorized;
}
void 
JabberHandler::LogOn() 
{
	if (fUsername != "" && fPassword != "" && fPort && fHost != "") 
	{
		// socket 0 is quite valid
		if(fSocket < 0) 
			BeginSession();
		Authorize();
	}
}
/*
void
JabberHandler::Register() 
{
	EndSession();
	if (BeginSession()) 
	{
		BString xml;
		xml << "<iq type='set' to='" << fHost << "'>";
		xml << "<query xmlns='jabber:iq:register'>";
		xml << 	"<password>" << fPassword << "</password>";
		xml << 	"<username>" << fUsername << "</username>";
		xml <<  "</query></iq>";
    	Send(xml);
	}
}
*/
void
JabberHandler::Register(JabberAgent * agent)
{
	BString xml;
	xml << "<iq type='get' to='" << agent->GetJid() << "'>";
	xml << "<query xmlns='jabber:iq:register'>";
	xml <<  "</query></iq>";
   	Send(xml);
}

void
JabberHandler::Register(JabberRegistration * registration)
{
	BString xml;
	JabberRegistration::FieldList * fields = registration->GetFields();
//	const BList* fields = pRegistration->GetFields();
	
	xml << "<iq type='set' to='" << registration->GetJid() << "'>";
	xml << "<query xmlns='jabber:iq:register'>";

	JabberRegistration::FieldList::iterator i;
	for (i = fields->begin(); i != fields->end(); i++) 
	{
		JabberRegistration::FieldPair pair = (*i);
		if (pair.first != "" && pair.second != "")
			xml << "<" << pair.first << ">" << pair.second << "</" << pair.first << ">";
		else if (pair.first != "")
			xml << "<" << pair.first << "/>";
	}
	xml << "</query></iq>";

   	Send(xml);
	return;
}

int32
JabberHandler::ReceivedData(const char * data,int32 length)  {
	
	if(length > 0){
		
		if (!XML_Parse(fParser, data, length, 0))
			logmsg("parse failed");
	}
	else
	
	if(IsAuthorized())
			Disconnected("Disconnected when receiving");
			
	
	return 0;
}

/*int32 
JabberHandler::GetConnection() { 
	return fPlug->GetConnection(); 
}
*/
void
JabberHandler::Send(const BString & xml) 
{
	if(fPlug->Send(xml) < 0 )
			Disconnected("Could not send");
}

void
JabberHandler::Authorize() 
{
//	CSHA1 sha1;
	BString xml;
//	char shaPassword[256];

//	sha1.Reset();	
//	sha1.Update((unsigned char*)mPassword->String(), mPassword->Length());
//	sha1.Final();
//	sha1.ReportHash(shaPassword, CSHA1::REPORT_HEX);

	
	xml	<< "<iq id='auth' type='set'>";
	xml <<		"<query xmlns='jabber:iq:auth'>";
	xml << 			"<username>" << fUsername << "</username>";
	xml << 			"<password>" << fPassword << "</password>";
//	xml << 			"<digest>" << shaPassword << "</digest>"; // Error encoding the password??
	xml << 			"<resource>" << fResource << "</resource>";
	xml << 		"</query>";
	xml << 	"</iq>";

	Send(xml);
}

bool
JabberHandler::BeginSession() 
{
	BString xml;
	
	if(fUsername == "" || fHost == "" || fPassword == "" || fPort <= 0) return false;
	
	fSocket = fPlug->StartConnection(fHost,fPort,this);
	
	if (fSocket >= 0) 
	{
		xml << "<stream:stream to=\'" << fHost << "\' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'>\n";
		Send(xml);
		return true;
	}
	else
	{
    	logmsg("failed to connect");
    	Disconnected("Failed to connect to host.");
    	return false;
    }
    
	return false;
}

void
JabberHandler::EndSession() 
{
	if(fSocket >= 0) 
	{
		BString xml;
		xml << "</stream:stream>\n";
		Send(xml);

		fSocket = -1;
		XML_ParserFree(fParser);
		fParser = XML_ParserCreate(NULL);
		XML_SetUserData(fParser, this);
		XML_SetElementHandler(fParser, StartElement, EndElement);
		XML_SetCharacterDataHandler(fParser, Characters);
		
		// TODO: Possibly empty all the lists?
	}
}


void
JabberHandler::StartElement(void * pUserData, const char * pName, const char ** pAttr) 
{
	BString name(pName);
	JabberHandler * handler = reinterpret_cast<JabberHandler *>(pUserData);
	
	// Authorize in the beginning of the stream
	if(name.ICompare("stream:stream") == 0) {
		//handler->Authorize();
		return;
	}
	if (name.ICompare("iq") == 0) 
	{
		const char * type = handler->HasAttribute("type", pAttr);
		const char * id = handler->HasAttribute("id", pAttr);
		if (type != NULL && 
		 	strcmp(type, "result") == 0 && 
		 	id != NULL && 
		 	strcmp(id, "auth") == 0) 
		 {
			handler->fAuthorized = true;
			handler->Authorized();
		}
	}

	JabberElement * element = new JabberElement;
	element->SetName(pName);
	element->SetAttr(pAttr);
	handler->fElementStack->AddItem(element);
	
	if (name.ICompare("query") == 0) 
	{
		const char * ns = handler->HasAttribute("xmlns", pAttr,element->GetAttrCount());
		if (ns != NULL) 
		{
			handler->fNsStack->push_back(BString(ns));	// the ns stack is an std::list
		}
	}
}

void
JabberHandler::EndElement(void * pUserData, const char * pName) 
{
	
	
	BString name(pName);
	JabberHandler * handler = (JabberHandler *)pUserData;
	
	//by xed
	JabberElement * element=handler->fElementStack->LastItem();
	if(element && element->GetName().ICompare("new_data")==0)
	 element->SetName("data");
	
	
	if (name.ICompare("message") == 0) 
	{
		JabberMessage *message = handler->BuildMessage();
		handler->Message(message);
	} 
	else if (name.ICompare("presence") == 0) 
	{
		JabberPresence *presence = handler->BuildPresence();
		if (presence->GetType().ICompare("subscribe") == 0)
		{
			handler->SubscriptionRequest(presence);
		} 
		else
		if (presence->GetType().ICompare("unsubscribe") == 0) //by xeD
		{
			handler->Unsubscribe(presence);
		} 
		else  
		{
			handler->Presence(presence);
			handler->UpdateRoster(presence);
		}
	} 
	else if (name.ICompare("query") == 0) 
	{
		if (handler->fNsStack->size() != 0)
		{
			BString obj = *(handler->fNsStack->begin());
			handler->fNsStack->pop_front();	// removes first item
			
			if (!obj.ICompare("jabber:iq:roster"))
			{
				handler->BuildRoster();
				handler->Roster(handler->fRoster);
			}
			else if (!obj.ICompare("jabber:iq:version"))
			{
				handler->SendVersion();
			}
			else if (!obj.ICompare("jabber:iq:agents"))
			{
				handler->BuildAgents();
				handler->Agents(handler->fAgents);
			}
			else if (!obj.ICompare("jabber:iq:register"))
			{
				JabberRegistration * r = handler->BuildRegistration();
				handler->Registration(r);
				delete r;
			}
		}
	}
}

void
JabberHandler::Characters(void * pUserData, const char * pString, int pLen) 
{
	
	
	JabberHandler * handler = (JabberHandler *)pUserData;
	JabberElement * element=handler->fElementStack->LastItem();
	
	char tmpz[pLen + 1];
	memcpy(tmpz, pString, pLen);
	tmpz[pLen] = 0;
		
	if(!element || element->GetName().ICompare("new_data")!=0) 
	{
		if(pLen==1 && (pString[0]=='\n' || pString[0]==9 || pString[0]==32 )) return;
		element = new JabberElement;
		element->SetName("new_data");
		//logmsg("Creating string %s",tmpz);
		element->SetData(tmpz);
		handler->fElementStack->AddItem(element);
	}
	
	else
	
	if(element->GetName().ICompare("new_data")==0)
	{
		BString tmp;
		tmp << element->GetData().String() << tmpz ;
		//logmsg("Adding string %s",tmp.String());
		element->SetData(tmp.String());
	}
	
	
	
}

/************************************************
 * Builds a message from the element stack
 ************************************************/
JabberMessage* 
JabberHandler::BuildMessage() 
{
	JabberMessage* message=new JabberMessage();
	JabberElement * element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	JabberElement * previous = NULL;
	StrList data;
	
	while (element->GetName().ICompare("message") != 0) 
	{
	
		if (element->GetName().ICompare("body") == 0) 
		{
			
			if (previous != 0 && previous->GetName().ICompare("data") == 0) 
			{
				message->SetBody(previous->GetData());
			}
			
		}
		else
		if (element->GetName().ICompare("error") == 0) 
		{
			
			if (previous != 0 && previous->GetName().ICompare("data") == 0) 
			{
				message->SetError(previous->GetData());
			}
			
		}
		else
		if (element->GetName().ICompare("x") == 0) 
		{
			// some 'x' features by xeD
			
			BString xmlns(HasAttribute("xmlns", element->GetAttr(),element->GetAttrCount()));
			logmsg("GetAttr xmlns #%s#",xmlns.String());
			message->SetX(xmlns);
			
			if (previous != 0)  
			{
				logmsg("previous->GetName() [%s] [%s]",previous->GetName().String(),previous->GetData().String());
				
				if (previous->GetName().ICompare("composing") == 0) 
				{
					//what should we do?
					// replace the X
					message->SetX("composing");			
				}
				else 
				//old olmeki compatibily..
				if(previous->GetName().ICompare("data") == 0){
				 if(previous->GetData().ICompare("Offline Storage") == 0)
					message->SetOffline(true);
										
				}	
			}
			
						
		}
		
		
		
		if (previous)
			delete previous;
		
		previous = element;
		element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);	
	}
	
	BString from(HasAttribute("from", element->GetAttr(),element->GetAttrCount()));
	message->SetID(HasAttribute("id", element->GetAttr(),element->GetAttrCount()));
	message->SetType(HasAttribute("type", element->GetAttr(),element->GetAttrCount()));
	
	StripResource(from);
	message->SetFrom(from);
	message->SetTo(GetJid());
	if (message->GetStamp() == "") 
		TimeStamp(*message);

	delete previous;
	delete element;
	
	//Temp the 'x' tag??
	
	//message->PrintToStream();
	
	//if(message->GetBody().ICompare("")==0) return NULL;	
	return message;
}

/************************************************************
 * Builds and/or updates the roster from elements on stack
 ************************************************************/
JabberHandler::RosterList * 
JabberHandler::BuildRoster() 
{
	JabberContact * contact = NULL;
	JabberElement * element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	JabberElement * previous = NULL;
	
	while (element->GetName().ICompare("query") != 0) 
	{
		contact = new JabberContact;
		while (element->GetName().ICompare("item") != 0) 
		{
			if (element->GetName().ICompare("group") == 0 && previous && previous->GetName().ICompare("data") == 0) 
			{
				contact->SetGroup(previous->GetData());
			}
			if (previous) 
				delete previous;
			previous = element;
			element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
		}

		BString jid(HasAttribute("jid", element->GetAttr(),element->GetAttrCount()));
		StripResource(jid);

		contact->SetJid(jid);
		contact->SetSubscription(HasAttribute("subscription", element->GetAttr(),element->GetAttrCount()));

		const char * tmpValue;
		tmpValue = HasAttribute("name", element->GetAttr(),element->GetAttrCount());
		tmpValue != NULL ? contact->SetName(tmpValue) : contact->SetName(contact->GetJid());

		
		if (contact->GetGroup() == NULL) 
		{
			const JabberAgent * agent = IsAgent(contact->GetJid());
			if (agent) 
			{
				contact->SetGroup("Transports");
				contact->SetName(agent->GetName());
			}
			else if (contact->GetSubscription() == "" || contact->GetSubscription().ICompare("from") == 0)
				contact->SetGroup("Unsubscribed");
			else
				contact->SetGroup("Unsorted");
		}
		
		UpdateRoster(contact);

		if (previous) 
			delete previous;
		previous = element;
		element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	}
	
	delete element;
	delete previous;
	return fRoster;
}

/******************************************************************
 * Builds a nice little precense object from the elements on stack
 ******************************************************************/
JabberPresence* 
JabberHandler::BuildPresence() 
{
	JabberElement * element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	JabberElement * previous = 0;
	JabberPresence *presence=new JabberPresence();

	presence->SetShow(S_ONLINE);
	while (element->GetName().ICompare("presence") != 0) 
	{
		if (element->GetName().ICompare("show") == 0) 
		{
			if (previous != NULL && previous->GetName().ICompare("data") == 0)
				presence->SetShowFromString(previous->GetData());
			//	logmsg("BUILD show %s",previous->GetData().String());
		} 
		else if (element->GetName().ICompare("status") == 0)
		{
			if (previous != NULL && previous->GetName().ICompare("data") == 0)
				presence->SetStatus(previous->GetData());
			//	logmsg("BUILD status %s",previous->GetData().String());
		}

		if (previous)
			delete previous;
		
		previous = element;
		element	= fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);	
	}
	
	BString from(HasAttribute("from", element->GetAttr(),element->GetAttrCount()));
	presence->ParseFrom(from);
	presence->SetType(HasAttribute("type", element->GetAttr(),element->GetAttrCount()));	

	delete previous;
	delete element;
	return presence;
}

/******************************************************************
 * Builds a list of available agents on the server, this is called
 * as a respond to the RequestAgents()
 *****************************************************************/
JabberHandler::AgentList *
JabberHandler::BuildAgents()
{
	JabberAgent * agent;
	JabberElement * element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	JabberElement * previous = NULL;
	
	while (element->GetName().ICompare("query") != 0) 
	{
		agent = new JabberAgent;
		while(element->GetName().ICompare("agent") != 0) 
		{
			if (element->GetName().ICompare("service") == 0 && previous->GetName().ICompare("data") == 0) 
			{
				agent->SetService(previous->GetData());
			}
			if (element->GetName().ICompare("name") == 0 && previous->GetName().ICompare("data") == 0) 
			{
				agent->SetName(previous->GetData());
			}
			if (element->GetName().ICompare("groupchat")) 
			{
				agent->SetGroupChat(true);
			}
			if (element->GetName().ICompare("search")) 
			{
				agent->SetSearchable(true);
			}
			if (element->GetName().ICompare("transport")) 
			{
				agent->SetTransport(true);
			}
			if (element->GetName().ICompare("register")) 
			{
				agent->SetRegistration(true);
			}
			
			if (previous) 
				delete previous;
			previous = element;
			element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);	
		}
		
		agent->SetJid(HasAttribute("jid", element->GetAttr(),element->GetAttrCount()));
		
		fAgents->AddItem(agent);
		
		if(previous) 
			delete previous;
		
		previous = element;
		element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);	
	}
	
	delete element;
	delete previous;
	return fAgents;
}

/*********************************************************
 * Builds a JabberRegistration object from elements on the
 * element stack
 *********************************************************/
JabberRegistration *
JabberHandler::BuildRegistration()
{
	JabberElement * element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	JabberElement * previous = NULL;
	JabberRegistration* registration = new JabberRegistration;

	while (element->GetName().ICompare("query") != 0) 
	{
		if (element->GetName().ICompare("instructions") == 0) 
		{
			if (previous != NULL && previous->GetName().ICompare("data") == 0)
				registration->SetInstructions(previous->GetData());
		} 
		else if(element->GetName().ICompare("data") != 0)
		{
				if (previous != NULL && previous->GetName().ICompare("data") == 0)
					registration->SetFieldValue(element->GetName().String(), previous->GetData().String(), true);
				else
					registration->AddField(element->GetName(), "");	// no value
		}
		
		if (previous)
			delete previous;
		
		previous = element;
		element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);
	}
	
	delete element;
	// TODO: what if CountItems() == 0? :) This applies to all uses of CountItems()
	// It will crash RemoveItem() (passing a -1)
	element = fElementStack->RemoveItemAt(fElementStack->CountItems() - 1);	
	registration->SetJid(HasAttribute("from", element->GetAttr()));

	delete previous;
	delete element;
	return registration;

}

void
JabberHandler::SendVersion() 
{
}

void
JabberHandler::RequestRoster() 
{
	BString xml;
	xml << "<iq id='roster' type='get'><query xmlns='jabber:iq:roster'/></iq>";
	Send(xml);
}

void
JabberHandler::RequestAgents() 
{
	BString xml;
	xml << "<iq type='get' to='" << fHost << "'>";
	xml <<	"<query xmlns='jabber:iq:agents'/>";
	xml << "</iq>";
	Send(xml);
}
void
JabberHandler::UpdateRoster(JabberPresence * presence) 
{
	JabberContact * contact;
	int size = fRoster->CountItems();
	
	for (int i = 0; i < size; i++) 
	{
		contact = fRoster->ItemAt(i);
		
		if (presence->GetJid().ICompare(contact->GetJid()) == 0) 
		{
			contact->SetPresence(presence);
			return;
		}
	}
	
	contact = new JabberContact();
	contact->SetJid(presence->GetJid());
	contact->SetName(presence->GetJid());	
	contact->SetPresence(presence); 
	fRoster->AddItem(contact);
}

void 
JabberHandler::UpdateRoster(JabberContact * contact) 
{
	JabberContact * current;
	int size = fRoster->CountItems();
	
	for (int i=0; i < size; i++) 
	{
		current = fRoster->ItemAt(i);
		
		if (contact->GetJid().ICompare(current->GetJid()) == 0) 
		{
			current->SetName(contact->GetName());
			current->SetGroup(contact->GetGroup());
			
			if(contact->GetSubscription() == "remove"){
			
				fRoster->RemoveItem(current);
				//delete current;
				delete contact;
				return;
			}
			else
			if(contact->GetSubscription() != "")
				current->SetSubscription(contact->GetSubscription());
			
			delete contact;
			return;
		}
	}
	fRoster->AddItem(contact);
}

/*********************
 * Utility methods
 *********************/
const JabberAgent *
JabberHandler::IsAgent(const BString & jid)
{
	JabberAgent * agent;
	int32 nbrAgents = fAgents->CountItems();
	
	for (int32 i = 0; i < nbrAgents; i++) 
	{
		 agent = fAgents->ItemAt(i);
		 if (jid.ICompare(agent->GetJid()) == 0)
		 	return agent;
	}
	
	return NULL;
}

void
JabberHandler::TimeStamp(JabberMessage & message) 
{
	BString tmp;
	BString timeString;
	
	time_t currentTime = time(NULL);
	struct tm * t = localtime(&currentTime);

	timeString << (t->tm_year+1900);
	TwoDigit(t->tm_mon, timeString);
	TwoDigit(t->tm_mday, timeString);
	timeString << "T";
	TwoDigit(t->tm_hour, timeString);	
	timeString << ":";	
	TwoDigit(t->tm_min, timeString);	
	timeString << ":";	
	TwoDigit(t->tm_sec, timeString);			

	message.SetStamp(timeString);
}

BString 
JabberHandler::TwoDigit(int32 number, BString & string) 
{
	if(number < 10)
		string << "0" << number;
	else
		string << number;

	return string;
}

const char * 
JabberHandler::HasAttribute(const char * pName, const char ** pAttributes,int32 count) 
{
	//logmsg("** Has Attribute for %s",pName);
	for (int32 i=0;i<count; i += 2) 
	{
		//logmsg("     %ld  -  %s",i,pAttributes[i]);
		if (strcmp(pAttributes[i], pName) == 0)
			{
				//logmsg("compare ok! val %s",pAttributes[i + 1]);
				return pAttributes[i + 1];
			}
	}
	//logmsg("");
	return NULL;
}
const char * 
JabberHandler::HasAttribute(const char * pName, const char ** pAttributes) 
{
	
	for (int32 i=0; pAttributes[i]; i += 2) 
	{
		if (strcmp(pAttributes[i], pName) == 0)
					return pAttributes[i + 1];
		
	}
	
	return NULL;
}
void
JabberHandler::StripResource(BString & jid) 
{
	int i = jid.FindFirst('/');
	if(i != -1)
		jid.Remove(i, jid.Length() - i);
}



/***********************************
 * Callbacks
 ***********************************/
void 
JabberHandler::Authorized() 
{
	/*
	 * Important that we get the list of agents first so that we 
	 * can add agents in the roster to a certain group
	 */
	RequestAgents(); 
	RequestRoster();
}





