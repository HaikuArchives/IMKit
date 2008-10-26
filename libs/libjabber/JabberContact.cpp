#include "JabberContact.h"
#include "stdio.h"

JabberContact::JabberContact() 
{
	fPresence = new JabberPresence();
	fId = "";
}

JabberContact::~JabberContact() 
{
}
void
JabberContact::PrintToStream() 
{
	
	printf("\nJabberContact\n");
	printf("     Name:  %s\n",fName.String());
	printf("    Group:  %s\n",fGroup.String());
	printf("      Jid:  %s\n",fJid.String());

	
	
}

void
JabberContact::SetName(const BString & name) 
{
	fName = name;
}

void
JabberContact::SetGroup(const BString & group) 
{
	fGroup = group;
}

void
JabberContact::SetPresence(JabberPresence * presence) 
{
	
	//presence->PrintToStream();
	//delete fPresence;
	fPresence = presence;
}

void
JabberContact::SetPresence()
{
	//fPresence->PrintToStream();
	delete fPresence;
	fPresence = new JabberPresence();	// construct an empty JabberPresence
}

void
JabberContact::SetJid(const BString & jid) 
{
	fJid = jid;
}

void
JabberContact::SetSubscription(const BString & subscription) 
{
	fSubscription = subscription;
}

BString
JabberContact::GetSubscription() const
{
	return fSubscription;
}

BString
JabberContact::GetName() const
{
	return fName;
}

BString
JabberContact::GetGroup() const
{
	return fGroup;
}

JabberPresence*
JabberContact::GetPresence()
{
	return fPresence;
}

BString
JabberContact::GetJid() const
{
	return fJid;
}
