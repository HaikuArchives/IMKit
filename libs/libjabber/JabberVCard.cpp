/*
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */

#include "JabberVCard.h"


JabberVCard::JabberVCard()
{
}


JabberVCard::JabberVCard(const JabberVCard& copy)
{
	SetFullName(copy.GetFullName());
	SetGivenName(copy.GetGivenName());
	SetFamilyName(copy.GetFamilyName());
	SetMiddleName(copy.GetMiddleName());
	SetNickname(copy.GetNickname());
	SetEmail(copy.GetEmail());
}


void
JabberVCard::operator=(const JabberVCard& vcard)
{
	if (this == &vcard)
		return;

	SetFullName(vcard.GetFullName());
	SetGivenName(vcard.GetGivenName());
	SetFamilyName(vcard.GetFamilyName());
	SetMiddleName(vcard.GetMiddleName());
	SetNickname(vcard.GetNickname());
	SetEmail(vcard.GetEmail());
}


void
JabberVCard::ParseFrom(const BString& from)
{
	fJid = "";
	fResource = "";

	int32 i = from.FindFirst('/');
	if (i != -1) {
		from.CopyInto(fJid, 0, i);
		from.CopyInto(fResource, i + 1, from.Length());
	} else
		fJid = from;
}


BString
JabberVCard::GetFullName() const
{
	return fFullName;
}


BString
JabberVCard::GetGivenName() const
{
	return fGivenName;
}


BString
JabberVCard::GetFamilyName() const
{
	return fFamilyName;
}


BString
JabberVCard::GetMiddleName() const
{
	return fMiddleName;
}


BString
JabberVCard::GetNickname() const
{
	return fNickname;
}


BString
JabberVCard::GetEmail() const
{
	return fEmail;
}


BString
JabberVCard::GetURL() const
{
	return fURL;
}


BString
JabberVCard::GetBirthday() const
{
	return fBirthday;
}


void
JabberVCard::SetFullName(const BString& firstName)
{
	fFullName = firstName;
}


void
JabberVCard::SetGivenName(const BString& name)
{
	fGivenName = name;
}


void
JabberVCard::SetFamilyName(const BString& name)
{
	fFamilyName = name;
}


void
JabberVCard::SetMiddleName(const BString& name)
{
	fMiddleName = name;
}


void
JabberVCard::SetNickname(const BString& name)
{
	fNickname = name;
}


void
JabberVCard::SetEmail(const BString& email)
{
	fEmail = email;
}


void
JabberVCard::SetURL(const BString& url)
{
	fURL = url;
}


void
JabberVCard::SetBirthday(const BString& birthday)
{
	fBirthday = birthday;
}
