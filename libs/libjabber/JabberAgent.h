#ifndef JABBER_AGENT_H
#define JABBER_AGENT_H

#include <String.h>

class JabberAgent 
{
public:
						JabberAgent();
						~JabberAgent();

	bool 				HasGroupChat();
	bool 				Searchable();
	bool 				IsTransport();
	bool 				AllowsRegistration();
	
	BString 			GetService() const;
	BString 			GetName() const;
	BString 			GetJid() const;		
	BString				GetInstructions() const;
		void				PrintToStream();
	
	void 				SetGroupChat(bool groupChat);
	void 				SetSearchable(bool searchable);
	void 				SetTransport(bool transport);
	void 				SetRegistration(bool registration);
	void 				SetService(const BString & service);
	void 				SetName(const BString & name);
	void 				SetJid(const BString & jid);		

private:
	bool 				fGroupChat;
	bool 				fSearchable;
	bool 				fTransport;
	bool 				fRegistration;
	BString 			fService;
	BString				fName;
	BString 			fJid;
};

#endif
