/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */

#include <stdio.h>

#include "JabberContact.h"

JabberContact::JabberContact()
	: fPresence(new JabberPresence()),
	fId("")
{
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
JabberContact::SetName(const BString& name) 
{
	fName = name;
}


void
JabberContact::SetGroup(const BString& group) 
{
	fGroup = group;
}


void
JabberContact::SetPresence(JabberPresence* presence) 
{
	fPresence = presence;
}


void
JabberContact::SetPresence()
{
	delete fPresence;
	fPresence = new JabberPresence();
}


void
JabberContact::SetJid(const BString& jid) 
{
	fJid = jid;
}


void
JabberContact::SetSubscription(const BString& subscription) 
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
			

BString
JabberContact::GetLastMessageID() const
{
	return fId;
}


void
JabberContact::SetLastMessageID(const BString& id)
{
	fId = id;
}
