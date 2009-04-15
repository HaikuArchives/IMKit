#include "OSCARConnection.h"

#include "common/BufferReader.h"
#include "FLAP.h"
#include "SNAC.h"
#include "TLV.h"

#include <UTF8.h>

#ifdef BONE_BUILD
	#include <sys/select.h>
	#ifdef __HAIKU__
		#define SHUTDOWN_BOTH SHUT_RDWR
	#endif
#else
	#include <netdb.h>
	#include <support/Locker.h>
	#include <support/Autolock.h>
#endif

void remove_html(char *msg);
const char *kThreadName = "IM Kit: OSCAR Connection";

OSCARConnection::OSCARConnection(const char *server, uint16 port, OSCARManager *man,
	const char *name = "OSCAR Connection", conn_type type = connBOS)
	: BLooper(name) {
	
	// Setup the map between SNAC families and the handling function
	fHandler[SERVICE_CONTROL] = &OSCARConnection::HandleServiceControl;
	fHandler[LOCATION] = &OSCARConnection::HandleLocation;
	fHandler[BUDDY_LIST_MANAGEMENT] = &OSCARConnection::HandleBuddyList;
	fHandler[ICBM] = &OSCARConnection::HandleICBM;
	fHandler[ADVERTISEMENTS] = &OSCARConnection::HandleAdvertisement;
	fHandler[INVITATION] = &OSCARConnection::HandleInvitation;
	fHandler[ADMINISTRATIVE] = &OSCARConnection::HandleAdministrative;
	fHandler[POPUP_NOTICES] = &OSCARConnection::HandlePopupNotice;
	fHandler[PRIVACY_MANAGEMENT] = &OSCARConnection::HandlePrivacy;
	fHandler[USER_LOOKUP] = &OSCARConnection::HandleUserLookup;
	fHandler[USAGE_STATS] = &OSCARConnection::HandleUsageStats;
	fHandler[TRANSLATION] = &OSCARConnection::HandleTranslation;
	fHandler[CHAT_NAVIGATION] = &OSCARConnection::HandleChatNavigation;
	fHandler[CHAT] = &OSCARConnection::HandleChat;
	fHandler[DIRECTORY_USER_SEARCH] = &OSCARConnection::HandleUserSearch;
	fHandler[SERVER_STORED_BUDDY_ICONS] = &OSCARConnection::HandleBuddyIcon;
	fHandler[SERVER_SIDE_INFORMATION] = &OSCARConnection::HandleSSI;
	fHandler[ICQ_SPECIFIC_EXTENSIONS] = &OSCARConnection::HandleICQ;
	fHandler[AUTHORISATION_REGISTRATION] = &OSCARConnection::HandleAuthorisation;
	
	fSockMsgr = NULL;
	fManager = man;
	fManMsgr = BMessenger(fManager);
	
	fConnType = type;
	fServer = server;
	fPort = port;
	fThread = B_ERROR;
	
	fRunner = new BMessageRunner(BMessenger(NULL, (BLooper *)this),
		new BMessage(AMAN_PULSE), 250000, -1);	

	fKeepAliveRunner = new BMessageRunner(BMessenger(NULL, (BLooper *)this),
		new BMessage(AMAN_KEEP_ALIVE), 30000000, -1);
	
#ifndef BONE_BUILD
	fSocketLock = new BLocker();
#endif
	
	SetState(OSCAR_CONNECTING);
	fSock = B_ERROR;
	fSock = ConnectTo(fServer.String(), fPort);
	if (fSock > B_OK) StartReceiver();
};

OSCARConnection::~OSCARConnection(void) {
	snooze(1000);
	
	StopReceiver();
	ClearQueue();
	
#ifndef BONE_BUILD
	delete fSocketLock;
#endif
};

//#pragma mark -

status_t OSCARConnection::Send(Flap *f, send_time at = atBuffer) {
	status_t status = B_OK;
	switch (at) {
		case atBuffer: {
			fOutgoing.push_back(f);
		} break;
		
		case atImmediate: {
			status = LowLevelSend(f);
		} break;

		case atOnline: {
			if (State() == OSCAR_ONLINE) {
				fOutgoing.push_back(f);
			} else {
				fOutgoingOnline.push_back(f);
			};
		} break;
		
		default: {
			status = B_ERROR;
		} break;
	};
	
	return status;
}

void OSCARConnection::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case AMAN_PULSE: {
			if (fOutgoing.size() == 0) return;
			Flap *flap = fOutgoing.front();
			LowLevelSend(flap);
		
			fOutgoing.pop_front();
		} break;
		
		case AMAN_KEEP_ALIVE: {
			if ((State() == OSCAR_ONLINE) || (State() == OSCAR_AWAY)) {
				Flap *keepAlive = new Flap(KEEP_ALIVE);
				Send(keepAlive);
			};
		} break;
		
		case AMAN_GET_SOCKET: {	
			BMessage reply(B_REPLY);
			reply.AddInt32("socket", fSock);
			msg->SendReply(&reply);
		} break;
		
		case AMAN_FLAP_OPEN_CON: {
//			We don't do anything with this currently
			const uchar *data;
			int32 bytes = 0;
			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
			LOG(ConnName(), liLow, "%s:%i: Got FLAP_OPEN_CON packet", Server(),
				Port());
		} break;
		
		case AMAN_FLAP_SNAC_DATA: {
			status_t result = kUnhandled;
			const uchar *data;
			int32 bytes = 0;
			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);
			
			BufferReader reader(data, bytes);
			SNAC snac(&reader);
			reader.OffsetTo(0);
			
			uint16 family = snac.Family();
			uint16 subtype = snac.SubType();

			LOG(ConnName(), liLow, "OSCARConn(%s:%i): Got SNAC (0x%04x, 0x%04x)",
				Server(), Port(), family, subtype);		

			handler_t::iterator hIt = fHandler.find(family);
			if (hIt != fHandler.end()) {
				FamilyHandler handler = hIt->second;
				result = (this->*handler)(&snac, &reader);
			} else {
				LOG(ConnName(), liHigh, "OSCARConn(%s:%i): Got a SNAC (0x%04x, 0x%04x) that "
					"had no handler\n", Server(), Port(), family, subtype);
			};
			if (result == kUnhandled) fManMsgr.SendMessage(msg);	
		} break;
			
		case AMAN_FLAP_ERROR: {
//			We ignore this for now
		} break;
				
		case AMAN_FLAP_CLOSE_CON: {
			const uchar *data;
			int32 bytes = 0;
			msg->FindData("data", B_RAW_TYPE, (const void **)&data, &bytes);

			if (State() == OSCAR_CONNECTING) {
	
				char *server = NULL;
				char *cookie = NULL;
				char *value = NULL;

				int32 i = 0;
				uint16 port = 0;
				uint16 cookieSize = 0;
	
				uint16 type = 0;
				uint16 length = 0;
	
				while (i < bytes) {
					type = (data[i] << 8) + data[++i];
					length = (data[++i] << 8) + data[++i];
					value = (char *)calloc(length + 1, sizeof(char));
					memcpy(value, (char *)(data + i + 1), length);
					value[length] = '\0';
	
					switch (type) {
						case 0x0001: {	// Our name, god knows why
						} break;
						
						case 0x0005: {	// New Server:IP
							pair<char *, uint16> serv = ExtractServerDetails(value);
							server = serv.first;
							port = serv.second;
							LOG(ConnName(), liLow, "Need to reconnect to: %s:%i", server, port);
						} break;
	
						case 0x0006: {
							cookie = (char *)calloc(length, sizeof(char));
							memcpy(cookie, value, length);
							cookieSize = length;
						} break;
					};
	
					free(value);
					i += length + 1;
					
				};
				
				fManager->Progress("OSCAR Login", "OSCAR: Reconnecting to server",
					0.25);
	
				StopReceiver();
				
//				Nuke the old packet queue
				ClearQueue();

				LOG(ConnName(), liDebug, "%s:%i attempting connection to %s:%i\n",
					fServer.String(), fPort, server, port);
				if (fSock > B_OK) {
					fSock = ConnectTo(server, port);

					fPort = port;
					fServer = server;
					
					free(server);
					
					StartReceiver();
					
					Flap *f = new Flap(OPEN_CONNECTION);
					f->AddInt32(0x0000001);	// ID
					f->AddTLV(0x0006, cookie, cookieSize);
					
					Send(f);
				} else {
					free(server);
				};
			} else {
				BMessage msg(AMAN_CLOSED_CONNECTION);
				msg.AddPointer("connection", this);

				fManMsgr.SendMessage(&msg);
				SetState(OSCAR_OFFLINE);

				StopReceiver();
				fSock = -1;
			};
		} break;
		
		default: {
			BLooper::MessageReceived(msg);
		};
	};
};

uint8 OSCARConnection::SupportedSNACs(void) const {
	return fSupportedSNACs.size();
};

uint16 OSCARConnection::SNACAt(uint8 index) const {
	return fSupportedSNACs[index];
};

bool OSCARConnection::Supports(const uint16 family) const {
	vector <const uint16>::iterator i;
		
	for (i = fSupportedSNACs.begin(); i != fSupportedSNACs.end(); i++) {
		if (family == (*i)) return true;
	};

	return false;
};

void OSCARConnection::Support(uint16 family) {
	fSupportedSNACs.push_back(family);
};

status_t OSCARConnection::SetState(uint8 state) {
	status_t status = B_OK;
	if (state == OSCAR_ONLINE) {
		flap_stack::iterator pIt;
		for (pIt = fOutgoingOnline.begin(); pIt != fOutgoingOnline.end(); pIt++) {
			fOutgoing.push_back(*pIt);
		};
		
		fOutgoingOnline.clear();
	};
	
	fState = state;
	
	return status;
};

ServerAddress OSCARConnection::ExtractServerDetails(char *details) {
	char *colon = strchr(details, ':');
	uint16 port = atoi(colon + 1);
	char *server = (char *)calloc((colon - details) + 1, sizeof(char));
	strncpy(server, details, colon - details);
	server[(colon - details)] = '\0';
	
	ServerAddress p(server, port);
	return p;
};

//#pragma mark -

status_t OSCARConnection::LowLevelSend(Flap *flap) {
	int sent_data = -1;

	if (fSock > 0) {
#ifndef BONE_BUILD
		BAutolock autolock(fSocketLock);
		if (autolock.IsLocked() == false) return B_ERROR;
#endif
		if (flap->Channel() == SNAC_DATA) {
			LOG(ConnName(), liLow, "%s:%i Sending 0x%04x / 0x%04x", Server(),
				Port(), flap->SNACAt()->Family(), flap->SNACAt()->SubType());
		};
		const char * data = flap->Flatten(++fOutgoingSeqNum);
		int32 data_size = flap->FlattenedSize();
		int32 sent_data = 0;
		
		LOG(ConnName(), liDebug, "%s:%i: Sending %ld bytes of data", Server(),
			Port(), data_size);
				
		while (sent_data < data_size) {
			int32 sent = send(fSock, &data[sent_data], data_size-sent_data, 0);
			
			if (sent < 0) {
				delete flap;
				LOG(ConnName(), liLow, "%s:%i: Couldn't send packet", Server(),
					Port());
				perror("Socket error1");
				return sent;
			};
			
			if (sent == 0) {
				LOG(ConnName(), liHigh, "%s:%i: send() returned 0, is this bad?",
					Server(), Port());
				snooze(1*1000*1000);
			};
			
			sent_data += sent;
		};
		
		LOG(ConnName(), liDebug, "%s:%i: Sent %ld bytes of data", Server(), Port(),
			data_size);
		
		delete flap;
	};
	
	return sent_data;
};

void OSCARConnection::ClearQueue(void) {
	flap_stack::iterator it;
	
	for (it = fOutgoing.begin(); it != fOutgoing.end(); it++) {
		Flap *f = (*it);
		delete f;
	};

	fOutgoing.clear();
};

int32 OSCARConnection::ConnectTo(const char *hostname, uint16 port) {
	if (hostname == NULL) {
		LOG(ConnName(), liHigh, "ConnectTo() called with NULL hostname - probably"
			" not authorised to login at this time");
		fManager->Error("Authorisation rejected");
		
		StopReceiver();
		
		BMessage msg(AMAN_CLOSED_CONNECTION);
		msg.AddPointer("connection", this);

		fManMsgr.SendMessage(&msg);
		SetState(OSCAR_OFFLINE);

		return B_ERROR;
	};

	struct hostent *he;
	struct sockaddr_in their_addr;
	int32 sock = 0;

	LOG(ConnName(), liLow, "OSCARConn::ConnectTo(%s, %i)", hostname, port);

	if ((he = gethostbyname(hostname)) == NULL) {
		LOG(ConnName(), liMedium, "OSCARConn::ConnectTo: Couldn't get Server name (%s)", hostname);
		return B_ERROR;
	};

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		LOG(ConnName(), liMedium, "OSCARConn::ConnectTo(%s, %i): Couldn't create socket",
			hostname, port);	
		return B_ERROR;
	};

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(port);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), 0, 8);

#ifndef BONE_BUILD
	BAutolock autolock(fSocketLock);
	if (autolock.IsLocked() == false) return B_ERROR;
#endif

	if (connect(sock, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
		LOG(ConnName(), liMedium, "OSCARConn::ConnectTo: Couldn't connect to %s:%i", hostname, port);

		StopReceiver();

		BMessage msg(AMAN_CLOSED_CONNECTION);
		msg.AddPointer("connection", this);

		fManMsgr.SendMessage(&msg);
		SetState(OSCAR_OFFLINE);
				
		return B_ERROR;
	};
	
	SetState(OSCAR_CONNECTING);
	
	return sock;
};

void OSCARConnection::StartReceiver(void) {
	fSockMsgr = new BMessenger(NULL, (BLooper *)this);

	fThread = spawn_thread(Receiver, kThreadName, B_NORMAL_PRIORITY, (void *)this);
	if (fThread > B_ERROR) resume_thread(fThread);
	
};

void OSCARConnection::StopReceiver(void) {
	if (fSock > B_ERROR) {
#ifndef BONE_BUILD
		closesocket(fSock);
#else
		shutdown(fSock, SHUTDOWN_BOTH);
		close(fSock);
#endif		
	};
	
	BMessenger * old_msgr = NULL;
	if (fSockMsgr) {
		old_msgr = fSockMsgr;
		fSockMsgr = new BMessenger((BHandler *)NULL);
		// deleting the messenger will cause the thread to exit cleanly
	};
	
	if (fThread > B_ERROR) {
		int32 res;
		if (wait_for_thread(fThread, &res) != B_OK) {
			LOG(ConnName(), liHigh, "Error waiting for thread!");
		};
		fThread = B_ERROR;
	};
	
	if (old_msgr != NULL) {
		delete old_msgr;
	};
};

int32 OSCARConnection::Receiver(void *con) {
	OSCARConnection *connection = reinterpret_cast<OSCARConnection *>(con);

	const uint8 kFLAPHeaderLen = 6;
	const uint32 kSleep = 2000000;
	const char *kHost = connection->Server();
	const uint16 kPort = connection->Port();
	const char *kConnName = connection->ConnName();
	
	int32 socket = 0;

	if ( !connection->fSockMsgr->IsValid() ) {
		LOG(kConnName, liLow, "%s:%i: Messenger wasn't valid!", kHost, kPort);
		return B_ERROR;
	}

	BMessage reply;

	status_t ret = 0;

	if ((ret = connection->fSockMsgr->SendMessage(AMAN_GET_SOCKET, &reply)) == B_OK) 
	{
		if ((ret = reply.FindInt32("socket", &socket)) != B_OK) 
		{
			LOG(kConnName, liLow, "%s:%i: Couldn't get socket: %i", kHost, kPort, ret);
			return B_ERROR;
		}
	} else 
	{
		LOG(kConnName, liLow, "%s:%i: Couldn't obtain socket: %i", kHost, kPort, ret);
		return B_ERROR;
	}
	
	if (socket < 0) {
		LOG(kConnName, liLow, "%s:%i: Socket is invalid: %i", kHost, kPort, socket);
		return B_ERROR;
	};
	
	struct timeval timeout;
	struct fd_set read;
	struct fd_set error;
	int16 bytes = 0;
	int32 processed = 0;
	uint16 flapLen = 0;
	uchar flapHeader[kFLAPHeaderLen];
	
	while (connection->fSockMsgr->IsValid() == true) {
		bytes = 0;
		processed = 0;
						
		while (processed < kFLAPHeaderLen) {
#ifndef BONE_BUILD
			BAutolock autolock(connection->fSocketLock);
#endif
			FD_ZERO(&read);
			FD_ZERO(&error);
			
			FD_SET(socket, &read);
			FD_SET(socket, &error);
			
			timeout.tv_sec = 1;
			
			if (select(socket + 1, &read, NULL, &error, &timeout) > 0) {
				if (FD_ISSET(socket, &error)) {
					LOG(kConnName, liLow, "%s:%i: Got socket error", kHost, kPort);
					snooze(kSleep);
					continue;
				};
				if (FD_ISSET(socket, &read)) {
					if ((bytes = recv(socket, (void *)(flapHeader + processed),
						kFLAPHeaderLen - processed, 0)) > 0) {
						processed += bytes;
					} else {
						if (bytes == 0) {
							if (connection->fSockMsgr->IsValid() == false) {
								// Messenger is no longer valid abort
								return B_OK;
							} else {
								snooze(kSleep);
								continue;
							};
						} else {
							if (connection->fSockMsgr->IsValid() == false) {
								// Shutting down
								return B_OK;
							};
							
							LOG(kConnName, liLow, "%s:%i: Socket got less than 0",
								kHost, kPort);
							perror("SOCKET ERROR");
							
							BMessage msg(AMAN_CLOSED_CONNECTION);
							msg.AddPointer("connection", con);
							
							connection->fManMsgr.SendMessage(&msg);
							connection->SetState(OSCAR_OFFLINE);
							
							return B_ERROR;
						};
					};					
				};
			} else {
				if (connection->fSockMsgr->IsValid() == false) {
					return B_OK;
				};
			};
		};
		
		uint8 channel = 0;
		uint16 seqNum = 0;
		uint16 flapLen = 0;
		uchar *flapContents;
		
		if (flapHeader[0] != COMMAND_START) {
			LOG(kConnName, liHigh, "%s:%i: Packet header doesn't start with 0x2a "
				" - discarding!", kHost, kPort);
			continue;
		};
		
		channel = flapHeader[1];
		seqNum = (flapHeader[2] << 8) + flapHeader[3];
		flapLen = (flapHeader[4] << 8) + flapHeader[5];
		
		flapContents = (uchar *)calloc(flapLen, sizeof(char));
		
		processed = 0;
		bytes = 0;
		
		while (processed < flapLen) {
#ifndef BONE_BUILD
			BAutolock autolock(connection->fSocketLock);
#endif
			FD_ZERO(&read);
			FD_ZERO(&error);
			
			FD_SET(socket, &read);
			FD_SET(socket, &error);
			
			timeout.tv_sec = 1;
			
			if (select(socket + 1, &read, NULL, &error, &timeout) > 0) {
				if (FD_ISSET(socket, &read)) {
					if ((bytes = recv(socket, (void *)(flapContents + processed),
						flapLen - processed, 0)) > 0) {
						processed += bytes;
					} else {
						if (bytes == 0) {
							if (connection->fSockMsgr->IsValid() == false) {
								return B_OK;
							} else {
								snooze(kSleep);
								continue;
							};
						} else {
							free(flapContents);
							if (connection->fSockMsgr->IsValid() == false) return B_OK;
							
							LOG(kConnName, liLow, "%s:%i. Got socket error:",
								connection->Server(), connection->Port());
							perror("SOCKET ERROR");
							
							BMessage msg(AMAN_CLOSED_CONNECTION);
							msg.AddPointer("connection", con);
							
							connection->fManMsgr.SendMessage(&msg);
							connection->SetState(OSCAR_OFFLINE);
							
							return B_ERROR;
						};
					};
				};
			} else {
				if (connection->fSockMsgr->IsValid() == false) {
					return B_OK;
				};
			};
		};
		
		BMessage dataReady;
		
		switch (channel) {
			case OPEN_CONNECTION: {
				dataReady.what = AMAN_FLAP_OPEN_CON;
			} break;
			case SNAC_DATA: {
				dataReady.what = AMAN_FLAP_SNAC_DATA;
			} break;
			case FLAP_ERROR: {
				dataReady.what = AMAN_FLAP_ERROR;
			} break;
			case CLOSE_CONNECTION: {
				dataReady.what = AMAN_FLAP_CLOSE_CON;
			} break;
			default: {
				LOG(kConnName, liHigh, "%s:%i Got an unsupported FLAP channel (%i)",
					kHost, kPort, channel);
				continue;
			};
		}
		
		dataReady.AddInt8("channel", channel);
		dataReady.AddInt16("seqNum", seqNum);
		dataReady.AddInt16("flapLen", flapLen);
		dataReady.AddData("data", B_RAW_TYPE, flapContents, flapLen);
		
		connection->fSockMsgr->SendMessage(&dataReady);
		
		free(flapContents);
	}
	
//	delete msgr;
	LOG(connection->fManager->Protocol(), liDebug, "Receiver thread exiting");
	
	return B_OK;
};

//#pragma mark SNAC Handlers

status_t OSCARConnection::HandleServiceControl(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleLocation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleBuddyList(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleICBM(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleAdvertisement(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleInvitation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleAdministrative(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandlePopupNotice(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandlePrivacy(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleUserLookup(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleUsageStats(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleTranslation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleChatNavigation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleChat(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleUserSearch(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleBuddyIcon(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleSSI(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleICQ(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};

status_t OSCARConnection::HandleAuthorisation(SNAC *snac, BufferReader *reader) {
	return kUnhandled;
};
