#include "Connection.h"

#include <stdio.h>
#include <string.h>

#include <Node.h>
#include <Path.h>
#include <TypeConstants.h>

#include "Helpers.h"

using namespace IM;

//#pragma mark Constructor

Connection::Connection(void)
	: fConn("") {
};

Connection::Connection(const char * str)
	: fConn(str) {

	int32 protoColon = fConn.FindFirst(":");
	int32 accountColon = fConn.FindFirst(":", protoColon + 1);

	fConn.CopyInto(fProtocol, 0, protoColon);
	if (accountColon != B_ERROR) {
		fConn.CopyInto(fAccount, protoColon + 1, accountColon - protoColon - 1);
		fConn.CopyInto(fID, accountColon + 1, fConn.Length() - accountColon);
	} else {
		fConn.CopyInto(fID, protoColon + 1, fConn.Length() - protoColon - 1);
	};
};

Connection::Connection(const char *protocol, const char *account, const char *ID)
	: fProtocol(protocol),
	fID(ID),
	fAccount(account) {

	fConn << fProtocol << ":";
	if (fAccount.Length() > 0) {
		fConn << fAccount << ":";
	};
	fConn << fID;
};

Connection::Connection(const Connection &c)
	: fConn(c.fConn),
	fProtocol(c.fProtocol),
	fID(c.fID),
	fAccount(c.fAccount) {
}

//#pragma mark Public

const char *Connection::Protocol(void) const {
	return fProtocol.String();
};

const char *Connection::ID(void) const {
	return fID.String();
};

const char *Connection::Account(void) const {
	return fAccount.String();
};

bool Connection::HasAccount(void) const {
	return (fAccount.Length() > 0);
};

const char *Connection::String(void) const {
	return fConn.String();
};

bool Connection::operator == (const Connection &rhs) const {
	return (fConn == rhs.fConn);
};
