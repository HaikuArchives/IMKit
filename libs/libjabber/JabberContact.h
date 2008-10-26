#ifndef JABBER_CONTACT_H
#define JABBER_CONTACT_H

#include <String.h>

#include "JabberPresence.h"

class JabberContact 
{
public:
						JabberContact();
	virtual				~JabberContact();
						
	void 				SetName(const BString & name);
	void 				SetGroup(const BString & group);
	void 				SetSubscription(const BString & group);
	virtual void		SetPresence();	// sets an empty presence
	virtual void 		SetPresence(JabberPresence * presence);
	void 				SetJid(const BString & jid);
	void				PrintToStream();
	BString 			GetName() const;
	BString 			GetGroup() const;
	BString 			GetSubscription() const;
	JabberPresence*		GetPresence();
	BString 			GetJid() const;

	BString				GetLastMessageID() const {	return fId; };
	void				SetLastMessageID(const BString & id) {	fId=id; };
		
private:
	BString				fJid;
	BString				fGroup;
	JabberPresence*		fPresence;
	BString				fName;
	BString				fId;
	BString				fSubscription;
};


#endif	// JABBER_CONTACT_H
