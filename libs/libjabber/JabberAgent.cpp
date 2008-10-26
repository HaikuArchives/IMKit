#include "JabberAgent.h"
#include "stdio.h"

JabberAgent::JabberAgent() 
{
	fGroupChat = fSearchable = fTransport = fRegistration = false;
	fService = fName = fJid = "";
}

void
JabberAgent::PrintToStream() 
{
	//by xed uses printf (no good)
	printf(" ** Jabber Agent **\n");
	printf("    Jid:  %s\n",fJid.String());
	printf("   name:  %s\n",fName.String());
	printf("service:  %s\n",fService.String());
		
	
}

JabberAgent::~JabberAgent()
{
}

bool
JabberAgent::HasGroupChat()
{
	return fGroupChat;
}

bool
JabberAgent::IsTransport()
{
	return fTransport;
}

bool
JabberAgent::Searchable()
{
	return fSearchable;
}

bool
JabberAgent::AllowsRegistration()
{
	return fRegistration;
}

BString
JabberAgent::GetService() const
{
	return fService;
}

BString
JabberAgent::GetName() const
{
	return fName;
}

BString
JabberAgent::GetJid() const
{
	return fJid;
}

void
JabberAgent::SetService(const BString & service)
{
	fService = service;
}

void
JabberAgent::SetName(const BString & name)
{
	fName = name;
}

void
JabberAgent::SetJid(const BString & jid)
{
	fJid = jid;
}

void
JabberAgent::SetGroupChat(bool groupChat)
{
	fGroupChat = groupChat;
}

void
JabberAgent::SetSearchable(bool searchable)
{
	fSearchable = searchable;
}

void
JabberAgent::SetTransport(bool transport)
{
	fTransport = transport;
}

void
JabberAgent::SetRegistration(bool registration)
{
	fRegistration = registration;
}
