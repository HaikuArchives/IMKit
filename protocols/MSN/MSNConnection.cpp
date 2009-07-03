#include "MSNConnection.h"

#include "MSNObject.h"
#include "SSO.h"

#include <openssl/md5.h>

//#pragma mark Constants

const char *kClientVer = "0x0407 win 6.0 i386 MSNMSGR 8.5 MSMSGS";
const char *kProtocolsVers = "MSNP15 CVR0";

const int16 kDefaultPort = 1863;
const uint32 kOurCaps = ccMultiPacketMessaging | ccMSNC8;

extern const char *kClientCode = "RG@XY*28Q5QHS%Q5";
extern const char *kClientID = "PROD0113H11T8$X_";

//#pragma mark Functions

void remove_html(char *msg);
void PrintHex(const unsigned char* buf, size_t size);

status_t SplitEmail(BString email, BString &username, BString &domain) {
	status_t result = B_ERROR;
	int at = email.FindFirst("@");
	
	if (at > B_ERROR) {
		email.MoveInto(username, 0, at);
		email.MoveInto(domain, 1, email.Length() - 1);
		
		result = B_OK;
	};

	return result;
}

//#pragma mark Constructor

MSNConnection::MSNConnection()
	: BLooper("MSNConnection looper") {

	fState = otOffline;
	fTrID = 0;
	fManager = NULL;
	fRunner = NULL;
	fKeepAliveRunner = NULL;
	fServer = NULL;
	fSock = -1;
	fSockMsgr = NULL;
	fThread = -1;
	
	SetupCommandHandlers();

	Run();
};

MSNConnection::MSNConnection(const char *server, uint16 port, MSNManager *man)
	: BLooper("MSNConnection looper") {
	
	fState = otOffline;
	fTrID = 0;
	fManager = NULL;
	fRunner = NULL;
	fKeepAliveRunner = NULL;
	fServer = NULL;
	fSock = -1;
	fSockMsgr = NULL;
	fThread = -1;
	
	SetupCommandHandlers();
	
	Run();
	SetTo(server,port,man);
};

MSNConnection::~MSNConnection(void) {
	if ( fRunner )
		delete fRunner;
	if ( fKeepAliveRunner )
		delete fKeepAliveRunner;
	
	if ( fServer )
		free(fServer);
//	snooze(1000);
	
	StopReceiver();
	
	if (fSock > 0) close(fSock);

	BMessage removeCon(msnmsgRemoveConnection), reply;
	removeCon.AddPointer("connection", this);
	
	// There's a slight risk with not having the reply here, I think.
	// At least there used to be, a race condition with the MSNManager
	// reading the connection's host and port.   /ME
	fManMsgr.SendMessage(&removeCon/*, reply*/);
};

//#pragma mark BLooper Hooks

void MSNConnection::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case msnmsgPulse: {
			if ((fState == otOnline) || (fState == otAway)) {
				CommandQueue::iterator i;
				for (i = fWaitingOnline.begin(); i != fWaitingOnline.end(); i++) {
					NetworkSend(*i);
				};
			};
			if (fOutgoing.size() == 0) return;
			
			Command *c = fOutgoing.front();
			NetworkSend(c);
			
			fOutgoing.pop_front();
		} break;

		case msnmsgPing: {
			if ((fState == otOnline) || (fState == otAway)) {
				Command *keepAlive = new Command("PNG");
				keepAlive->UseTrID(false);
				Send(keepAlive);
			};
		} break;
		
		case msnmsgGetSocket: {	
			BMessage reply(B_REPLY);
			reply.AddInt32("socket", fSock);
			msg->SendReply(&reply);
		} break;
		
		case msnmsgDataReady: {
			Command *command = NULL;
			if (msg->FindPointer(("command"), (void **)&command) == B_OK) {
				ProcessCommand(command);
				delete command;
			};
		} break;
		
		default: {
			BLooper::MessageReceived(msg);
		};
	};
};

bool MSNConnection::QuitRequested(void) {
	if (fState == otOnline) {
		// Leave gracefully if connected
		Command *bye = new Command("OUT");
		bye->UseTrID(false);
		Send(bye, qsImmediate);
	};
	
	return true;
};

//#pragma mark Accessors

const char *MSNConnection::Server(void) const {
	return fServer;
};

uint16 MSNConnection::Port(void) const {
	return fPort;
};

bool MSNConnection::IsConnected() {
	return fSock > B_ERROR;
};

//#pragma mark Public Methods

void MSNConnection::SetTo(const char *server, uint16 port, MSNManager *man) {
	fTrID = 0;
	fManager = man;
	fManMsgr = BMessenger(fManager);
	
	fServer = strdup(server);
	fPort = port;
	
	fRunner = new BMessageRunner(BMessenger(NULL, (BLooper *)this),
		new BMessage(msnmsgPulse), 500000, -1);

	fKeepAliveRunner = new BMessageRunner(BMessenger(NULL, (BLooper *)this),
		new BMessage(msnmsgPing), 60000000, -1);
	
	fState = otConnecting;
	fSock = ConnectTo(fServer, fPort);
	
	StartReceiver();
};

status_t MSNConnection::Send(Command *command, queuestyle queue = qsQueue) {
	status_t ret = B_ERROR;
	
	switch (queue) {
		case qsImmediate: {
			ret = NetworkSend(command);
		} break;
		
		case qsQueue: {
			fOutgoing.push_back(command);
			ret = B_OK;
		} break;
		
		case qsOnline: {
			fWaitingOnline.push_back(command);
			ret = B_OK;
		} break;
	};
	
	return ret;
};

//#pragma mark Protected

ServerAddress MSNConnection::ExtractServerDetails(char *details) {
	char *colon = strchr(details, ':');
	uint16 port = atoi(colon + 1);
	char *server = (char *)calloc((colon - details) + 1, sizeof(char));
	strncpy(server, details, colon - details);
	server[(colon - details)] = '\0';
	
	ServerAddress p(server, port);
	return p;
};

void MSNConnection::GoOnline(void) {
	CommandQueue::iterator i;

	for (i = fWaitingOnline.begin(); i != fWaitingOnline.end(); i++) {
		fOutgoing.push_back(*i);
	};
	fWaitingOnline.clear();
	
	fState = otOnline;
};

void MSNConnection::ClearQueues(void) {
	CommandQueue::iterator i;
	for (i = fOutgoing.begin(); i != fOutgoing.end(); i++) delete (*i);
	fOutgoing.clear();

	for (i = fWaitingOnline.begin(); i != fWaitingOnline.end(); i++) delete (*i);
	fWaitingOnline.clear();
};

status_t MSNConnection::ProcessCommand(Command *command) {
	status_t result = B_ERROR;
	commandhandler_t::iterator hIt = fCommandHandler.find(command->Type());
	LOG(kProtocolName, liDebug, "%lX received a \"%s\"\n", this, command->Type().String());
	
	if (hIt != fCommandHandler.end()) {
		CommandHandler handler = hIt->second;
		result = (this->*handler)(command);
	} else {
		LOG(kProtocolName, liHigh, "%lX got an unhandled command: \"%s\"\n", this,
			command->Type().String());
	};
	
	return result;
};


status_t MSNConnection::HandleVER( Command * /*command*/ ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing VER", this);
	Command *reply = new Command("CVR");
	reply->AddParam(kClientVer);
	reply->AddParam(fManager->Passport());
		
	Send(reply);
		
	return B_OK;
}

status_t MSNConnection::HandleNLN( Command * command ) {
//	XXX - We should probably look at the caps and nick	
	BString statusStr = command->Param(0);
	BString passport = command->Param(1);
	BString friendly = command->Param(2);
	BString caps = command->Param(3);

	buddymap *buddies = fManager->BuddyList();
	buddymap::iterator it = buddies->find(passport);
	Buddy *buddy;
	if (it == buddies->end()) {
		buddy = new Buddy(passport.String());
		buddies->insert(pair<BString, Buddy*>(passport, buddy));
	} else {
		buddy = it->second;
	};
	
	online_types status = otOffline;
	
	if (statusStr == "NLN") status = otOnline;
	if (statusStr == "BSY") status = otBusy;
	if (statusStr == "IDL") status = otIdle;
	if (statusStr == "BRB") status = otBRB;
	if (statusStr == "AWY") status = otAway;
	if (statusStr == "PHN") status = otPhone;
	if (statusStr == "LUN") status = otLunch;

	buddy->Status(status);
	buddy->Capabilities(atol(caps.String()));
	buddy->FriendlyName(friendly.String());

	if (command->Params() > 4) {
		BString obj = command->Param(4, true);
		buddy->DisplayPicture(new MSNObject(obj.String(), obj.Length()));
	};
	
	BMessage statusChange(msnmsgStatusChanged);
	statusChange.AddString("passport", passport);
	statusChange.AddInt8("status", (int8)status);
	
	fManMsgr.SendMessage(&statusChange);
	return B_OK;
}

status_t MSNConnection::HandleCVR( Command * /*command*/ ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing CVR", this);
	Command *reply = new Command("USR");
	reply->AddParam("SSO");	// Auth type - Single Sign On
//	reply->AddParam("TWN");	// Authentication type
	reply->AddParam("I");	// Initiate
	reply->AddParam(fManager->Passport());
	
	Send(reply);
	
	return B_OK;
}

status_t MSNConnection::HandleRNG( Command * command ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing RNG", this);
//		The details are actually param 1, but param 0 will be interpretted as the
//		TrID
	LOG(kProtocolName, liDebug, "C %lX: got a chat invite from %s (%s)",
		this, command->Param(4), command->Param(3));
	ServerAddress sa = ExtractServerDetails((char *)command->Param(0));
	
	BMessage newCon(msnmsgNewConnection);
	newCon.AddString("host", sa.first);
	newCon.AddInt16("port", sa.second);
	newCon.AddString("type", "RNG");
	newCon.AddString("authType", command->Param(1));
	newCon.AddString("authString", command->Param(2));
	newCon.AddString("inviterPassport", command->Param(3));
	newCon.AddString("inviterDisplayName", command->Param(4));
	
	BString temp;
	temp << command->TransactionID();
	newCon.AddString("sessionID", temp.String() );
	
	fManMsgr.SendMessage(&newCon);
	
	return B_OK;
}

status_t MSNConnection::HandleXFR( Command * command ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing XFR", this);
	command->Debug();
	
	ServerAddress sa = ExtractServerDetails((char *)command->Param(1));
	
	BMessage newCon(msnmsgNewConnection);
	newCon.AddString("host", sa.first);
	newCon.AddInt16("port", sa.second);
	newCon.AddString("type", command->Param(0));
	newCon.AddInt32("trid", command->TransactionID());
	
	if (strcmp(command->Param(0), "NS") == 0) {
		StopReceiver();
			
		ClearQueues();
			
		BMessage closeCon(msnmsgCloseConnection);
		closeCon.AddPointer("connection", this);
			
		fManMsgr.SendMessage(&closeCon);
	};
		
	if (strcmp(command->Param(0), "SB") == 0) {
		newCon.AddString("authType", command->Param(2));
		newCon.AddString("authString", command->Param(3));
	};			

	fManMsgr.SendMessage(&newCon);

	return B_OK;
}

status_t MSNConnection::HandleCHL( Command * command ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing CHL", this);

	const char *challenge = command->Param(0);
	int i = 0;
	unsigned char buf[256];
	char chlString[128];
	long long high = 0;
	long long low = 0;
	long long temp = 0;
	long long key = 0;
	long long bskey = 0;
	int *chlStringArray = (int *)chlString;
	int *md5hash = (int *)buf;
	char hash1a[17];
	char hash2a[17];

	sprintf((char *)buf + 16, "%s%s", challenge, kClientCode);
	MD5(buf + 16, strlen((char *)buf + 16), buf);
	for (i = 0; i < 16; i++) {
		sprintf((char *)buf + 16 + i * 2,"%02x", buf[i]);
	};
	
	for (i = 0; i < 4; i++) {
	   md5hash[i] = md5hash[i] & 0x7FFFFFFF;
	};
	
	i = (strlen(challenge) + strlen(kClientID) + 7) & 0xF8;
	sprintf(chlString,"%s%s00000000", challenge, kClientID);
	chlString[i] = 0;
	
	for (i = 0; i < (int)strlen(chlString) / 4; i += 2) {
		temp = chlStringArray[i];
		
		temp = (0x0E79A9C1 * temp) % 0x7FFFFFFF;
		temp += high;
		temp = md5hash[0] * temp + md5hash[1];
		temp = temp % 0x7FFFFFFF;
		
		high = chlStringArray[i + 1];
		high = (high + temp) % 0x7FFFFFFF;
		high = md5hash[2] * high + md5hash[3];
		high = high % 0x7FFFFFFF;
		
		low = low + high + temp;
	};
	
	high = (high + md5hash[1]) % 0x7FFFFFFF;
	low = (low + md5hash[3]) % 0x7FFFFFFF;
	
	key = (low << 32) + high;
	for (i= 0; i < 8; i++) {
		bskey <<= 8;
		bskey += key & 255;
		key >>=8;
	};
	
	strncpy((char *)hash1a, (char *)buf + 16, 16);
	strncpy((char *)hash2a, (char *)buf + 32, 16);
	hash1a[16] = '\0';
	hash2a[16] = '\0';
	
	sprintf((char *)buf, "%llx%llx", strtoull(hash1a,NULL,16) ^ bskey,
		strtoull(hash2a,NULL,16) ^ bskey);

	Command *reply = new Command("QRY");
	reply->AddParam(kClientID);
	reply->AddPayload((char *)buf, 32);
	
	Send(reply);
	reply->Debug();
	
	return B_OK;
};

status_t MSNConnection::HandleUSR( Command * command ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing USR", this);
	status_t result = B_ERROR;

	if (strcmp(command->Param(0), "OK") == 0) {
		Progress("MSN Login", "MSN: Logged in!", 1.0);
		
		GoOnline();
		
		BMessage statusChange(msnmsgOurStatusChanged);
		statusChange.AddInt8("status", fState);
		fManMsgr.SendMessage(&statusChange);

		/*
		 * Depending on the server protocol, server location and the
		 * account being used (non-hotmail/msn acc), the server will not
		 * respond on the PRP MFN command and our connection will break.
		 * Michael
		 */
		BString passport(fManager->Passport());
		
		if ((B_ERROR != passport.FindFirst("@hotmail")) || (B_ERROR != passport.FindFirst("@msn"))) {
			LOG(kProtocolName, liDebug, "Sending PRP MSN command" );
			
			Command *rea = new Command("PRP");
			rea->AddParam("MFN");
			rea->AddParam(fManager->DisplayName(), true);
			Send(rea);
		} else {
			LOG(kProtocolName, liDebug, "Not sending PRP MSN command!");
		};
		
		fManager->Handler()->ContactList(&fContacts);
				
		Command *adl = new Command("ADL");
		
		list<BString> contacts;
		map<BString, list<BString> > domainUsers;
		
		fManager->Handler()->ContactList(&contacts);
		
		// Make our contact list into a list by domain
		for (list<BString>::iterator iIt = contacts.begin(); iIt != contacts.end(); iIt++) {
			BString user;
			BString domain;
			
			if (SplitEmail((*iIt).String(), user, domain) == B_OK) {			
				map<BString, list<BString> >::iterator dIt = domainUsers.find(domain);
				if (dIt == domainUsers.end()) {
					list<BString> users;
					users.push_back(user);
					
					domainUsers[domain] = users;
				} else {
					dIt->second.push_back(user);
				};
			};
		};

		BString payload = "<ml l=\"1\">";

		// Package each contact by domain
		for (map<BString, list<BString> >::iterator dIt = domainUsers.begin(); dIt != domainUsers.end(); dIt++) {
			payload << "<d n=\"" << dIt->first << "\">";
		
			for (list<BString>::iterator uIt = dIt->second.begin(); uIt != dIt->second.end(); uIt++) {
				payload << "<c n=\"" << (*uIt) <<  "\" l=\"3\" t=\"1\" />";
			};
			
			payload << "</d>";
		};
		
		payload << "</ml>";
		adl->AddPayload(payload.String());
		
		Send(adl);
		
		Command *chg = new Command("CHG");
		chg->AddParam("NLN");
		BString caps = "";
		caps << kOurCaps;
		chg->AddParam( caps.String() );
		Send(chg);

		result = B_OK;
	} else {
		BString URI = command->Param(2);
		BString nonce = command->Param(3);
		BString authResp = "";
		BString authToken = "";

		SSO *auth = new SSO(NULL, fManager->Passport(), fManager->Password(), URI.String(),
			nonce.String());

		result = auth->Response(authToken, authResp);

		if (result == B_OK) {
			Command *reply = new Command("USR");
			reply->AddParam("SSO");
			reply->AddParam("S");
			reply->AddParam(authToken.String());
			reply->AddParam(authResp.String());
			
			reply->Debug();
			
			Send(reply);
		};

	};
	
	return result;
}

status_t MSNConnection::HandleMSG( Command * command ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing MSG", this);
	
	HTTPFormatter http(command->Payload(0), strlen(command->Payload(0)));
	
	BString type;
	status_t result = http.HeaderContents("Content-Type", type);

	if (result == B_OK) {
		// Handle type
		if (type == "text/plain; charset=UTF-8") {
			LOG(kProtocolName, liHigh, "C %lX: Got a private message [%s] from <%s>", this, http.Content(), command->Param(0));
			fManager->fHandler->MessageFromUser(command->Param(0), http.Content());
		} else
		if (type == "text/x-msmsgscontrol")  {
			LOG(kProtocolName, liHigh, "C %lX: User typing from <%s>", this, command->Param(0));
			fManager->fHandler->UserIsTyping(http.HeaderContents("TypingUser"),	tnStartedTyping);
		} else
		if (type == "text/x-msmsgsinitialemailnotification; charset=UTF-8") {
			LOG(kProtocolName, liHigh, "C %lX: HotMail number of messages in Inbox", this );
			// Ignore this. It just tells us how many emails are in the Hotmail inbox.
		} else
		if (type == "text/x-msmsgsprofile; charset=UTF-8") {
			LOG(kProtocolName, liHigh, "C %lX: Profile message", this );
			// Maybe we should process this, it's a profile message.
		} else
		if (type == "text/x-msmsgsinvite; charset=UTF-8") {
			LOG(kProtocolName, liDebug, "C %lX: Got an invite, we're popular!", this);
			PrintHex((uchar *)command->Payload(0), strlen(command->Payload(0)));
		} else
		if (type == "application/x-msnmsgrp2p") {
			LOG(kProtocolName, liDebug, "C %lX: Got Peer To Peer message!", this);
		} else {
			LOG(kProtocolName, liDebug, "C %lX: Got message of unknown type <%s>", this, type.String());
			PrintHex((uchar *)command->Payload(0), strlen(command->Payload(0)));
		}
	} else {
		LOG(kProtocolName, liDebug, "C %lX: No Content-Type in message!", this);
	}
		
	return B_OK;
}

status_t MSNConnection::HandleADC(Command *command) {
	LOG(kProtocolName, liDebug, "C %lX: Processing ADC", this);
	
	BString listStr = command->Param(0);
	BString passport = command->Param(1, true);
	passport.ReplaceFirst("N=", "");
	BString display = "<unknown display name>";
	if ( command->Params() == 3 )
	{
		display = command->Param(2, true);
		display.ReplaceFirst("F=", "");
	}
	
	list_types listtype = ltReverseList;
	
	if (listStr == "RL") listtype = ltReverseList;
	if (listStr == "AL") listtype = ltAllowList;
	if (listStr == "FL") listtype = ltForwardList;
	if (listStr == "BL") listtype = ltBlockList;
	
	if ( listtype == ltReverseList )
	{
		BMessage requestAuth(msnAuthRequest);
		requestAuth.AddString("displayname", display);
		requestAuth.AddString("passport", passport);
		requestAuth.AddInt8("list", (int8)listtype);
		
		BMessenger(fManager).SendMessage(&requestAuth);
	}
	
	return B_OK;
};

status_t MSNConnection::HandleLST(Command *command) {
	LOG(kProtocolName, liDebug, "C %lX: Processing LST", this);
	
	BString passport = command->Param(0);
	passport.ReplaceFirst("N=", "");
	BString display = command->Param(1, true);
	display.ReplaceFirst("F=", "");
	
	BMessage contactInfo(msnContactInfo);
	contactInfo.AddString("passport", passport.String() );
	contactInfo.AddString("display", display.String() );
	
	if ( command->Params() == 4 )
	{ // this param might not be here if the contact is in no lists
		//	This is a bitmask. 1 = FL, 2 = AL, 4 = BL, 8 = RL
		int32 lists = atol(command->Param(3));
		
		LOG(kProtocolName, liDebug, "C %lX: %s (%s) is in list %i", this, passport.String(), display.String(), lists);
		
/*		if (lists == ltReverseList) {
			LOG(kProtocolName, liDebug, "C %lX: \"%s\" (%s) is only on our reverse list. Likely they "
				"added us while we were offline. Ask for authorisation", this, display.String(),
				passport.String());
			fManager->Handler()->AuthRequest(ltReverseList, passport.String(), display.String());
		}; */
		
		contactInfo.AddInt32("lists", lists);
	} else
	{
		LOG(kProtocolName, liDebug, "C %lX: %s (%s) is in no lists", this, passport.String(), display.String() );
		contactInfo.AddInt32("lists", 0);
	}
	
	fManMsgr.SendMessage( &contactInfo );
	
	return B_OK;
};

status_t MSNConnection::HandleQRY(Command * /*command*/) {
	// ignore this. It's a challenge response status indicator.
	LOG(kProtocolName, liDebug, "C %lX: Processing QRY", this);
	return B_OK;
};

status_t MSNConnection::HandleGTC(Command * /*command*/) {
	// ignore this. It's a setting for the client that dictates how
	// to Handle people added to the RL. See protocol spec, Notification,
	// Getting details, Privacy settings.
	// Eg: GTC {TrID} GTC N
	LOG(kProtocolName, liDebug, "C %lX: Processing GTC", this);
	return B_OK;
};

status_t MSNConnection::HandleBLP(Command * /*command*/) {
	// Default list for contact not on either BL or AL.
	// Eg: BLP {TrID} BLP AL
	LOG(kProtocolName, liDebug, "C %lX: Processing BLP", this);
	return B_OK;
};

status_t MSNConnection::HandlePRP(Command * /*command*/) {
	// Contact phone numbers
	LOG(kProtocolName, liDebug, "C %lX: Processing PRP", this);
	return B_OK;
};

status_t MSNConnection::HandleCHG(Command * command) {
	// Own status changed
	LOG(kProtocolName, liDebug, "C %lX: Processing CHG", this);
	
	BString status = command->Param(0);
	
	BMessage statusChange(msnmsgOurStatusChanged);
	
	if ( status == "NLN" ) {
		statusChange.AddInt8("status", otOnline);
	} else
	if ( status == "AWY" ) {
		statusChange.AddInt8("status", otAway);
	} else {
		LOG(kProtocolName, liDebug, "C %lX: Unknown status: %s", this, status.String() );
		statusChange.what = 0; // so we don't send the message.
	}
	
	if ( statusChange.what )
		fManMsgr.SendMessage(&statusChange);
	
	return B_OK;
};

status_t MSNConnection::HandleFLN(Command *command) {
	// Contact went offline
	LOG(kProtocolName, liDebug, "C %lX: Processing FLN", this);
	
	BMessage statusChange(msnmsgStatusChanged);
	statusChange.AddString("passport", command->Param(0) );
	statusChange.AddInt8("status", otOffline);
	fManMsgr.SendMessage(&statusChange);
	
	return B_OK;
};

//status_t MSNConnection::HandleSYN( Command * /*command*/ ) {
status_t MSNConnection::HandleSYN( Command * command ) {
	LOG(kProtocolName, liDebug, "C %lX: Processing SYN", this);
	
	// process SYN here as needed
	// ...
	
	// send CHG to set our status
	Command *reply = new Command("CHG");
	reply->AddParam("NLN");
	BString caps = "";
	caps << kOurCaps;
	reply->AddParam( caps.String() );
	
	Send(reply);
	
	return B_OK;
};

status_t MSNConnection::HandleJOI(Command * /*command*/) {
	// Someone joining conversation
	LOG(kProtocolName, liDebug, "C %lX: Processing JOI", this);
	return B_OK;
};

status_t MSNConnection::HandleCAL(Command * /*command*/) {
	// Inititation response
	LOG(kProtocolName, liDebug, "C %lX: Processing CAL", this);
	return B_OK;
};

status_t MSNConnection::HandleIRO(Command * /*command*/) {
	// people already in conversation
	LOG(kProtocolName, liDebug, "C %lX: Processing IRO", this);
	return B_OK;
};

status_t MSNConnection::HandleANS(Command * /*command*/) {
	// Fully connected to chat session
	LOG(kProtocolName, liDebug, "C %lX: Processing ANS", this);
	return B_OK;
};

status_t MSNConnection::HandleBYE(Command * /*command*/) {
	// someone left conversation
	LOG(kProtocolName, liDebug, "C %lX: Processing BYE", this);
	return B_OK;
};

status_t MSNConnection::HandleSBS(Command *command) {
	printf("SBS Command!\n");
	command->Debug();
	
	Command *reply = new Command("CHG");
	reply->AddParam("NLN");
	BString caps = "";
	caps << kOurCaps;
	reply->AddParam( caps.String() );
		
	return B_OK;
};

status_t MSNConnection::HandleILN(Command *command) {
	LOG(kProtocolName, liDebug, "C %1X: Processing ILN", this);
	command->Debug();
	
	// For now just forward the ILN command to NLN
	return HandleNLN(command);
};

//#pragma mark Private

void MSNConnection::SetupCommandHandlers(void) {
	fCommandHandler["VER"] = &MSNConnection::HandleVER;
	fCommandHandler["NLN"] = &MSNConnection::HandleNLN;
	fCommandHandler["CVR"] = &MSNConnection::HandleCVR;
	fCommandHandler["RNG"] = &MSNConnection::HandleRNG;
	fCommandHandler["XFR"] = &MSNConnection::HandleXFR;
	fCommandHandler["CHL"] = &MSNConnection::HandleCHL;
	fCommandHandler["USR"] = &MSNConnection::HandleUSR;
	fCommandHandler["MSG"] = &MSNConnection::HandleMSG;
	fCommandHandler["ADC"] = &MSNConnection::HandleADC;
	fCommandHandler["LST"] = &MSNConnection::HandleLST;
	fCommandHandler["QRY"] = &MSNConnection::HandleQRY;
	fCommandHandler["GTC"] = &MSNConnection::HandleGTC;
	fCommandHandler["BLP"] = &MSNConnection::HandleBLP;
	fCommandHandler["PRP"] = &MSNConnection::HandlePRP;
	fCommandHandler["CHG"] = &MSNConnection::HandleCHG;
	fCommandHandler["FLN"] = &MSNConnection::HandleFLN;
	fCommandHandler["SYN"] = &MSNConnection::HandleSYN;
	fCommandHandler["JOI"] = &MSNConnection::HandleJOI;
	fCommandHandler["CAL"] = &MSNConnection::HandleCAL;
	fCommandHandler["IRO"] = &MSNConnection::HandleIRO;
	fCommandHandler["ANS"] = &MSNConnection::HandleANS;
	fCommandHandler["BYE"] = &MSNConnection::HandleBYE;
	fCommandHandler["SBS"] = &MSNConnection::HandleSBS;
	fCommandHandler["ILN"] = &MSNConnection::HandleILN;
};

int32 MSNConnection::NetworkSend(Command *command) {
	if (fSock > 0) {
		const char * data = command->Flatten(++fTrID);
		int32 data_size = command->FlattenedSize();
		int32 sent_data = 0;
		
		// uncomment following to see exactly what's sent
/*		char * blah = (char*)malloc(data_size+1);
		memcpy(blah, data, data_size);
		blah[data_size] = 0;
		printf("MSN sending command: [%s]\n", blah);
		free(blah);*/
		
		while (sent_data < data_size) {
			int32 sent = send(fSock, &data[sent_data], data_size-sent_data, 0);
			
			if (sent <= 0) {
				delete command;
				LOG(kProtocolName, liLow, "C %lX: Couldn't send packet", this);
				perror("Send Error");
				return sent;
			};
			sent_data += sent;
		};
		
		delete command;
		return sent_data;
	};
	
	return B_ERROR;
};

int32 MSNConnection::ConnectTo(const char *hostname, uint16 port) {
	if (hostname == NULL) {
		LOG(kProtocolName, liLow, "C %lX: ConnectTo() called with NULL hostname - probably"
			" not authorised to login at this time", this);
		return B_ERROR;
	};

	int32 sock = 0;

	LOG(kProtocolName, liLow, "C %lX: MSNConn::ConnectTo(\"%s\", %i)", this, hostname, port);

	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;

//	XXX - BONE Specific
//	net_server equiviliant would be;
//	if ((int)(remoteAddr.sin_addr.s_addr = inet_addr(hostname)) <= 0) {
	if (inet_aton(hostname, &remoteAddr.sin_addr) == 0) {
		struct hostent *he = gethostbyname(hostname);
		if (he) {
			remoteAddr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
		} else {
			LOG(kProtocolName, liLow, "C %lX: MSNConn::ConnectTo(%s, %i) couldn't resolve",
				this, hostname, port);
			return B_ERROR;
		};
	};
    remoteAddr.sin_port = htons(port);
	memset(&(remoteAddr.sin_zero), 0, 8);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG(kProtocolName, liMedium, "C %lX: MSNConn::ConnectTo(%s, %i): Couldn't create socket",
			this, hostname, port);	
		return B_ERROR;
	};

	if (connect(sock, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) == -1) {
		LOG(kProtocolName, liMedium, "C %lX: MSNConn::ConnectTo: Couldn't connect to %s:%i", 
			this, hostname, port);
		return B_ERROR;
	};
	
	LOG(kProtocolName, liLow, "C %lX:   Connected.", this);

	return sock;
};

void MSNConnection::StartReceiver(void) {
	if ( fSock <= B_ERROR )
	{ // error!
		Error("Not connected", true);
		return;
	}
	
	fSockMsgr = new BMessenger(this);
	
	fThread = spawn_thread(Receiver, "MSN socket", B_NORMAL_PRIORITY, (void *)this);
	if (fThread > B_ERROR) {
		resume_thread(fThread);
		LOG(kProtocolName, liHigh, "C %lX: Started receiver thread", this);
	} else
		LOG(kProtocolName, liHigh, "C %lX: Error creating receiver thread in MSNConnection!", this);
};

void MSNConnection::StopReceiver(void) {
	if (fSockMsgr) {
		BMessenger * old_msgr = fSockMsgr;
		fSockMsgr = new BMessenger((BHandler *)NULL);
		delete old_msgr;
	};
	
	if (fThread > 0) {
//		the following causes a EINTR error in the reader thread, which will then
//		quit in a decent way.
		suspend_thread(fThread);
		resume_thread(fThread);
		int32 res=0;
		wait_for_thread(fThread, &res);
	}
	
	fThread = -1;
};

int32 MSNConnection::Receiver(void *con) {
	LOG(kProtocolName, liLow, "C[r] %lX: Receiver init", con);

	MSNConnection *connection = reinterpret_cast<MSNConnection *>(con);

	const uint32 kSleep = 2000000;
	BMessenger **kMsgr = &connection->fSockMsgr;
	int32 socket = 0;
	
	if ( !(*kMsgr)->IsValid() ) {
		LOG(kProtocolName, liLow, "C[r] %lX: Messenger wasn't valid!", connection);
		return B_ERROR;
	}
	
	BMessage reply;
	socket = connection->fSock;
		
	struct fd_set read;
	struct fd_set error;
	int16 bytes = 0;
	int32 processed = 0;
	char buffer[1024];
	BString commandBuff = "";
	int32 kNewLineLen = strlen("\r\n");
	
	LOG(kProtocolName, liLow, "C[r] %lX: Starting receiver loop", connection);
	
	while ((*kMsgr)->IsValid() == true) {
		FD_ZERO(&read);
		FD_ZERO(&error);
		
		FD_SET(socket, &read);
		FD_SET(socket, &error);

		memset(buffer, 0, sizeof(buffer));

		if (select(socket + 1, &read, NULL, &error, NULL) > 0) {

			if (FD_ISSET(socket, &error)) {
				LOG(kProtocolName, liLow, "C[r] %lX: Got socket error", connection);
				snooze(kSleep);
				continue;
			};
			
			if (FD_ISSET(socket, &read)) {
				bytes = recv(socket, buffer, sizeof(buffer), 0);
				if (bytes > 0) {
					commandBuff.Append(buffer, bytes);
					processed += bytes;
				} else {
					if ((*kMsgr)->IsValid() == false) return B_OK;
					
					LOG(kProtocolName, liLow, "C[r] %lX: Socket got less than 0 (or 0)",
						connection);
					perror("SOCKET ERROR");
					
					BMessage msg(msnmsgCloseConnection);
					msg.AddPointer("connection", con);
					
					connection->fManMsgr.SendMessage(&msg);
					connection->fState = otOffline;
					
					return B_ERROR;
				};					
			};
		};
		
		int32 commandLen = commandBuff.FindFirst("\r\n");
		
		while (commandLen > 0) {
			BString command = "";
			int32 totalLen = commandBuff.Length();
			commandBuff.MoveInto(command, 0, commandLen + kNewLineLen);

			processed = totalLen - (commandLen + kNewLineLen);

			Command *comm = new Command("");
			comm->MakeObject(command.String());
			int32 payloadLen = 0;

			if (comm->ExpectsPayload(&payloadLen) == true) {
				LOG(kProtocolName, liDebug, "C[r] %lX: Payload of %i, have %i bytes",
					connection, payloadLen, processed);		
				
				while (processed < payloadLen) {
					FD_ZERO(&read);
					FD_ZERO(&error);
					
					FD_SET(socket, &read);
					FD_SET(socket, &error);
			
					memset(buffer, 0, sizeof(buffer));
			
					if (select(socket + 1, &read, NULL, &error, NULL) > 0) {				
						if (FD_ISSET(socket, &error)) {
							LOG(kProtocolName, liLow, "C[r] %lX: Got socket error", 
								connection);
							snooze(kSleep);
							continue;
						};
						
						if (FD_ISSET(socket, &read)) {
							bytes = recv(socket, buffer, min_c(payloadLen - processed, (int32)sizeof(buffer)), 0);
							if (bytes > 0) {
								commandBuff.Append(buffer, bytes);
								processed += bytes;
							} else {
								if ( (*kMsgr)->IsValid() == false) return B_OK;
		
								LOG(kProtocolName, liLow, "--C[r] %lX: Socket got less than 0 (or 0: %i)",
									connection, bytes);
								perror("SOCKET ERROR");
		
								BMessage msg(msnmsgCloseConnection);
								msg.AddPointer("connection", con);
				
								connection->fManMsgr.SendMessage(&msg);
								connection->fState = otOffline;
								
								return B_ERROR;
							};					
						};
					};
					
				};
				
//				add payload to message?
				comm->AddPayload( commandBuff.String(), payloadLen, false );
				commandBuff.Remove( 0, payloadLen );
				
				processed = commandBuff.Length();
			};
			
			BMessage dataReady(msnmsgDataReady);
			dataReady.AddPointer("command", comm);
			
			(*kMsgr)->SendMessage(&dataReady);
			
// 			this line needed to process several commands at once if they're in the
//			buffer
			commandLen = commandBuff.FindFirst("\r\n");
		};
	};
	
	return B_OK;
};

void MSNConnection::Error(const char *message, bool disconnected) {
	BMessage msg( msnmsgError );
	msg.AddString("error", message);
	msg.AddBool("disconnected", disconnected);
	
	fManMsgr.SendMessage(&msg);
	
	if (disconnected) {
		BMessage disMsg(msnmsgCloseConnection);
		disMsg.AddPointer("connection", this);
		fManMsgr.SendMessage(&disMsg);
	};
};

void MSNConnection::Progress(const char *id, const char *message, float progress) {
	BMessage msg(msnmsgProgress);
	msg.AddString("id", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	
	fManMsgr.SendMessage(&msg);
};
