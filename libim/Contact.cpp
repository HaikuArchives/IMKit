#include "Contact.h"

#include <stdio.h>
#include <string.h>

#include <Node.h>
#include <Path.h>
#include <TypeConstants.h>

#include "Connection.h"
#include "Helpers.h"

using namespace IM;

//#pragma mark Contact

Contact::Contact()
	: fEntry(-1,-1, "")
{
}

Contact::~Contact()
{
	Clear();
}

Contact::Contact( const entry_ref & entry )
{
	SetTo(entry);
}

Contact::Contact( const BEntry & entry )
{
	SetTo(entry);
}

Contact::Contact( const Contact & contact )
{
	SetTo( contact.fEntry );
}

void
Contact::SetTo( const entry_ref & entry )
{
	fEntry = entry;
	Update();
}

void
Contact::SetTo( const BEntry & entry )
{
	entry.GetRef(&fEntry);
	Update();
}

void
Contact::SetTo( const Contact & contact )
{
	fEntry = contact.fEntry;
	Update();
}

status_t
Contact::InitCheck()
{

	BEntry entry(&fEntry);
	status_t ret = entry.InitCheck();
	if (ret == B_OK) {
		ret = (entry.Exists() == true) ? B_OK : B_ERROR;
		
/*		if (ret == B_OK) {
			LoadConnections();
			if (CountConnections() <= 0) ret = B_ERROR;
		};
*/	};
	
	return ret;
}

bool
Contact::Exists()
{
	BEntry entry(&fEntry);
	return entry.Exists();
}

void
Contact::Update()
{
	Clear();
}

/**
	Clear memory caches of any data read from attributes
*/
void
Contact::Clear()
{
	for (int32 i = 0; i < fConnections.CountItems(); i++) {
		delete (char*)fConnections.ItemAt(i);
	};
	fConnections.MakeEmpty();

	for (int32 i = 0; i < fGroups.CountItems(); i++) {
		delete (char *)fGroups.ItemAt(i);
	};
	fGroups.MakeEmpty();	
};

status_t Contact::LoadConnections(void) {
	Clear();
	
	BNode node(&fEntry);
	if (node.InitCheck() != B_OK) {
		return B_ERROR;
	};
	
	char attr[2048];
	
	int32 num_read = node.ReadAttr("IM:connections", B_STRING_TYPE, 0, attr, sizeof(attr));
	
	if (num_read <= 0) {
		return B_ERROR;
	};
	
	attr[num_read] = 0;
	
	// data read. process it.
	int32 start = 0, curr = 0;
	char * conn = new char[256];
	
	while (attr[curr]) {
		if (attr[curr] != ';') {
			conn[curr - start] = attr[curr];
		} else {
			// separator
			if (curr != start) {
				conn[curr - start] = 0;
				
				BString toLower(conn);
				toLower.ToLower();
				strcpy(conn, toLower.String());

				Connection connection(conn);
				if ((strlen(connection.Protocol()) > 0) && (strlen(connection.ID()) > 0)) {
					fConnections.AddItem(conn);
					conn = new char[256];
				};
				
				start = curr + 1;
			};
		};
		
		curr++;
	};
	
	if (start != curr) {
		conn[curr - start] = 0;

		BString toLower(conn);
		toLower.ToLower();
		strcpy(conn, toLower.String());

		Connection connection(conn);
		if ((strlen(connection.Protocol()) > 0) && (strlen(connection.ID()) > 0)) {
			fConnections.AddItem(conn);
		};
	} else {
		delete conn;
	}
	
	return B_OK;
};

/**
	Write list of connections to disk
*/
status_t
Contact::SaveConnections()
{
	char attr[2048];
	attr[0] = 0;
	
	for ( int i=0; i<fConnections.CountItems(); i++ )
	{
		strcat(attr,(char*)fConnections.ItemAt(i));
		strcat(attr,";");
	}
	
	BNode node(&fEntry);
	
	if ( node.InitCheck() != B_OK )
		return B_ERROR;
	
	if ( node.WriteAttr(
		"IM:connections", B_STRING_TYPE, 0,
		attr, strlen(attr) + 1
	) != (int32)strlen(attr) + 1)
	{ // error writing
		return B_ERROR;
	}
	
	return B_OK;
}

status_t
Contact::AddConnection( const char * proto_id )
{
	LOG("Contact", liLow, "Adding connection %s\n", proto_id);
	
	if ( fConnections.CountItems() == 0 )
		LoadConnections();
	
	char * copy = new char[strlen(proto_id)+1];
	
	strcpy(copy,proto_id);
	
	fConnections.AddItem(copy);
	
	return SaveConnections();
}

status_t
Contact::RemoveConnection( const char * proto_id )
{
	LOG("Contact", liLow, "Removing connection %s\n", proto_id);
	
	if ( fConnections.CountItems() == 0 )
		LoadConnections();
	
	for ( int i=0; i<fConnections.CountItems(); i++ )
	{
		if ( strcmp(proto_id,(char*)fConnections.ItemAt(i)) == 0 )
		{
			delete[] (char*)fConnections.ItemAt(i);
			fConnections.RemoveItem(i);
			
			return SaveConnections();
		}
	}
	
	return B_ERROR;
}

int
Contact::CountConnections()
{
	if ( fConnections.CountItems() == 0 )
		LoadConnections();
	
	return fConnections.CountItems();
}

status_t
Contact::ConnectionAt( int index, char * buffer )
{
	if ( fConnections.CountItems() == 0 )
		LoadConnections();
	
	if ( index >= CountConnections() )
		return B_ERROR;
	
	strcpy( buffer, (char*)fConnections.ItemAt(index) );
	
	return B_OK;
}

status_t Contact::FindConnection(const char * _protocol, char * buffer) {
	if ( fConnections.CountItems() == 0 )
		LoadConnections();
	
	BString str(_protocol);
	str.ToLower();
	
	string protocol(str.String());
	
	for ( int i=0; i < fConnections.CountItems(); i++ ) {
		const char *connBuffer = (const char *)fConnections.ItemAt(i);
		
		Connection con(connBuffer);
		if (protocol == con.Protocol()) {
			strcpy(buffer, connBuffer);
			return B_OK;
		};
	};
	
	return B_ERROR;
}

bool
Contact::operator == ( const entry_ref & entry ) const
{
	return fEntry == entry;
}

bool
Contact::operator == ( const BEntry & entry ) const
{
	entry_ref ref;
	
	entry.GetRef(&ref);
	
	return fEntry == ref;
}

entry_ref Contact::EntryRef(void) const {
	return fEntry;
};

bool
Contact::operator == ( const Contact & contact ) const
{
	return fEntry == contact.fEntry;
}

bool
Contact::operator < ( const Contact & contact ) const
{
	BEntry ea(&fEntry), eb(&contact.fEntry);
	BPath a, b;
	ea.GetPath(&a);
	eb.GetPath(&b);
	
	return strcmp( a.Path(), b.Path() ) < 0;
}

Contact::operator const entry_ref * () const
{
	return &fEntry;
}

status_t
Contact::SetStatus( const char * status )
{
	BNode node(&fEntry);
	
	if ( node.InitCheck() != B_OK )
		return B_ERROR;
	
	if ( node.WriteAttr(
		"IM:status", B_STRING_TYPE, 0,
		status, strlen(status)+1
	) != (int32)strlen(status)+1 )
	{
		return B_ERROR;
	}
	
	node.SetModificationTime( time(NULL) );
	
	node.Unset();
	
	return B_OK;
}

status_t
Contact::ReadAttribute( const char * attr, char *buffer, int bufsize )
{
	BNode node(&fEntry);
	
	if ( node.InitCheck() != B_OK )
		return B_ERROR;
	
	int32 num_read = node.ReadAttr(attr, B_STRING_TYPE, 0, buffer, bufsize );
	
	if ( num_read <= 0 )
		return B_ERROR;
	
	buffer[num_read] = 0;
	
	node.Unset();
	
	return B_OK;
}

status_t Contact::ReadAttribute(const char *name, char **buffer, int32 *size) {
	status_t ret = B_OK;
	BNode node(&fEntry);
	ret = node.InitCheck();
	if (ret == B_OK) {
		attr_info info;
		ret = node.GetAttrInfo(name, &info);
		if (ret == B_OK) {
			*buffer = (char *)calloc(info.size, sizeof(char));
			ret = node.ReadAttr(name, info.type, 0, *buffer, info.size);
			if (ret > B_OK) {
				*size = ret;
				ret = B_OK;
			} else {
				free(*buffer);
			};
		};
	};
	
	return ret;
};

status_t
Contact::GetName( char * buffer, int size )
{
	return ReadAttribute("META:name",buffer,size);
}

status_t
Contact::GetNickname( char * buffer, int size )
{
	return ReadAttribute("META:nickname",buffer,size);
}

status_t
Contact::GetEmail( char * buffer, int size )
{
	return ReadAttribute("META:name",buffer,size);
}

status_t
Contact::GetStatus( char * buffer, int size )
{
	return ReadAttribute("IM:status",buffer,size);
}

status_t Contact::SetBuddyIcon(const char *protocol, BBitmap *icon) {
	BMessage iconAttr;
	char *buffer = NULL;
	int32 size = -1;
	BMessage flattenedIcon;
	status_t ret = B_ERROR;
	BMessage iconMsg;

	if (icon->Archive(&flattenedIcon) != B_OK) return B_ERROR;
	
	if (ReadAttribute("IM:buddyicons", &buffer, &size) == B_OK) {
		iconAttr.Unflatten(buffer);
		free(buffer);
	};

	if (iconAttr.FindMessage(protocol, &iconMsg) == B_OK) {
		iconAttr.ReplaceMessage(protocol, &flattenedIcon);
	} else {
		iconAttr.AddMessage(protocol, &flattenedIcon);
	};

	size = iconAttr.FlattenedSize();
	buffer = (char *)calloc(size, sizeof(char));
	if (iconAttr.Flatten(buffer, size) == B_OK) {
		BNode node(&fEntry);
		ret = node.WriteAttr("IM:buddyicons", B_MESSAGE_TYPE, 0, buffer, size);
	};
		
	free(buffer);
	return ret;
};

BBitmap *Contact::GetBuddyIcon(const char *protocol, int16 /*size*/) {
	char *buffer = NULL;
	int32 length = -1;

	if (ReadAttribute("IM:buddyicons", &buffer, &length) == B_OK) {
		BMessage iconAttr;
		BMessage iconMsg;

		if (iconAttr.Unflatten(buffer) != B_OK) {
			free(buffer);
			return NULL;
		};
		free(buffer);
		
		if (iconAttr.FindMessage(protocol, &iconMsg) == B_OK) {
			return new BBitmap(&iconMsg);
		};
	};
	
	return NULL;
};

int32 Contact::CountGroups(void) {
	if (fGroups.CountItems() == 0) LoadGroups();
	
	return fGroups.CountItems();
};

const char *Contact::GroupAt(int32 index) {
	if (fGroups.CountItems() == 0) LoadGroups();
	const char *result = NULL;
	if (index <= fConnections.CountItems()) {
		result = (const char *)fGroups.ItemAt(index);
	};

	return result;
};

status_t Contact::LoadGroups(void) {
	char *buffer = NULL;
	int32 size = -1;
	int32 count = 0;
	status_t result = ReadAttribute("META:group", &buffer, &size);
	if (result != B_OK) return result;
	
	BString groups;
	groups.SetTo(buffer, size);
	free(buffer);

	while (groups.FindFirst(",") != B_ERROR) {
		int offset = groups.FindFirst(",");
		BString temp;
		temp.SetTo(groups, offset);
		
		char *group = new char[offset + 1];
		strncpy(group, temp.String(), offset);
		group[offset] = '\0';
		
		fGroups.AddItem(group);
		groups.Remove(0, offset + 1);
		count++;
	};
	
	if (groups.Length() > 0) {
		char *group = new char[size + 1];
		strncpy(group, groups.String(), size);
		group[size] = '\0';
		
		fGroups.AddItem(group);
	};
	
	return B_OK;
};
