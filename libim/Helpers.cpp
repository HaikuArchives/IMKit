#include "Helpers.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <Directory.h>
#include <Node.h>
#include <Entry.h>
#include <string.h>
#include <stdlib.h>
#include <Path.h>
#include <FindDirectory.h>

log_importance g_verbosity_level = liDebug;

bool _has_checked_for_tty = false;

void
check_for_tty()
{
	if ( _has_checked_for_tty )
		return;
	
	_has_checked_for_tty = true;
	
	if ( !isatty(STDOUT_FILENO) )
	{ // redirect output to ~/im_kit.log if not run from Terminal
		BPath path;
	
		if (find_directory(B_USER_DIRECTORY,&path,true,NULL) != B_OK)
			// This should never fail..
			return;
		
		path.Append("im_kit.log");
		
		close(STDOUT_FILENO);
		open( path.Path(), O_WRONLY|O_CREAT|O_APPEND|O_TEXT, 0600);
	}
}

bool _has_loaded_log_level = false;

void
load_log_level()
{
	if ( _has_loaded_log_level )
		return;
	
	_has_loaded_log_level = true;
	
	BMessage settings;
	if ( im_load_client_settings("im_server", &settings) != B_OK )
	{
		printf("load_log_level: im_server settings not found\n");
		return;
	}
	
	const char * level=NULL;
	if ( settings.FindString("log_level", &level) == B_OK )
	{
		// check for the various levels..
		if ( strcmp(level, "Debug") == 0 ) {
			g_verbosity_level = liDebug;
		} else
		if ( strcmp(level, "Low") == 0 ) {
			g_verbosity_level = liLow;
		} else
		if ( strcmp(level, "Medium") == 0 ) {
			g_verbosity_level = liMedium;
		} else
		if ( strcmp(level, "High") == 0 ) {
			g_verbosity_level = liHigh;
		} else
		if ( strcmp(level, "Quiet") == 0 ) {
			g_verbosity_level = liQuiet;
		} else
			g_verbosity_level = liHigh;
	}
}

const char * gLogLevelText[] = {
	"debug",
	"low",
	"medium",
	"high"
};

// Note: if you change something in this LOG,
// make sure to change the LOG below as the code
// unfortunately isn't shared. :/
void LOG(const char * module, log_importance level, const char *message, const BMessage *msg, ...) {
	va_list varg;
	va_start(varg, msg);
	char buffer[2048];
	vsprintf(buffer, message, varg);
	
	load_log_level();
	
	if ( level > liHigh )
	{
		level = liHigh;
	}
	
	if ( level < g_verbosity_level )
		return;
	
	check_for_tty();
	
	char timestr[64];
	time_t now = time(NULL);
	strftime(timestr,sizeof(timestr),"%Y-%m-%d %H:%M", localtime(&now) );
	
	printf("%s %s (%s): %s\n", module, timestr, gLogLevelText[level], buffer);
	if ( msg )
	{
		printf("BMessage for last message:\n");
		msg->PrintToStream();
	}
	
	fflush(stdout);
}

void LOG(const char * module, log_importance level, const char *message, ...) {
	va_list varg;
	va_start(varg, message);
	char buffer[2048];
	vsprintf(buffer, message, varg);
	
	load_log_level();
	
	if ( level > liHigh )
	{
		level = liHigh;
	}
	
	if ( level < g_verbosity_level )
		return;
	
	check_for_tty();
	
	char timestr[64];
	time_t now = time(NULL);
	strftime(timestr,sizeof(timestr),"%Y-%m-%d %H:%M", localtime(&now) );
	
	printf("%s %s (%s): %s\n", module, timestr, gLogLevelText[level], buffer);
	
	fflush(stdout);
}

// READ / WRITE ATTRIBUTES

status_t
im_load_attribute( const char * path, const char * attribute, BMessage * msg )
{
	const int32 kDataChunkSize = 128*1024;
	char * data = (char*)malloc(kDataChunkSize);
	
	BNode node( path );
	
	if ( node.InitCheck() != B_OK )
	{
		LOG("helpers", liLow, "load_attribute: Error opening file (%s)", attribute, path);
		free(data);
		return B_ERROR;
	}
	
	int32 num_read = node.ReadAttr(
		attribute, B_RAW_TYPE, 0,
		data, kDataChunkSize
	);
	
	if ( num_read <= 0 )
	{
		LOG("helpers", liLow, "load_attribute: Error reading (%s) from (%s) [%lX: %s]", 
			attribute, path, num_read, strerror(num_read) );
		node.Unset();
		free(data);
		return B_ERROR;
	}
	
	LOG("helpers", liLow, "load_attribute: Read %ld bytes of data", num_read );
	
	if ( msg->Unflatten(data) != B_OK )
	{
		LOG("helpers", liHigh, "ERROR: load_attribute: Error unflattening (%s) from (%s)", attribute, path);
		node.Unset();
		free(data);
		return B_ERROR;
	}
	
	LOG("helpers", liDebug, "Read (%s) from (%s)", attribute, path);
	
	node.Unset();
	
	free(data);
	return B_OK;
}

status_t
im_save_attribute( const char * path, const char * attribute, const BMessage * msg )
{
	// save settings
	int32 data_size=msg->FlattenedSize();
	
	char * data = (char*)malloc(data_size);

	if ( msg->Flatten(data,data_size) != B_OK )
	{ // error flattening message
		LOG("helpers", liHigh, "ERROR: save_attribute: Error flattening (%s) message for (%s)", attribute, path);
		free(data);
		return B_ERROR;
	}
	
	BDirectory dir;
	dir.CreateFile(path,NULL,true);
	
	BNode node( path );
	
	if ( node.InitCheck() != B_OK )
	{
		LOG("helpers", liHigh, "ERROR: save_attribute: Error opening save file (%s):(%s)", attribute, path);
		free(data);
		node.Unset();
		return B_ERROR;
	}
	
	int32 num_written = node.WriteAttr(
		attribute, B_RAW_TYPE, 0,
		data, data_size
	);
	
	free(data);
	
	if ( num_written != data_size )
	{ // error saving settings
		LOG("helpers", liHigh, "ERROR: save_attribute: Error saving (%s) to (%s)", attribute, path);
		node.Unset();
		return B_ERROR;
	}
	
	LOG("helpers", liDebug, "Wrote (%s) to file: %s", attribute, path);
	
	node.Unset();
	
	return B_OK;
}

// LOAD SETTINGS

status_t
im_load_settings( const char * path, BMessage * msg )
{
	return im_load_attribute( path, "im_settings", msg );
}

status_t
im_load_template( const char * path, BMessage * msg )
{
	return im_load_attribute( path, "im_template", msg );
}

status_t
im_load_protocol_settings( const char * protocol, BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/add-ons/protocols");
	path.Append( protocol );
		
	return im_load_settings( path.Path(), msg );
}

status_t
im_load_protocol_template( const char * protocol, BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/add-ons/protocols");
	path.Append( protocol );
		
	return im_load_template( path.Path(), msg );
}

status_t
im_load_client_settings( const char * client, BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/clients");
	path.Append( client );
		
	return im_load_settings( path.Path(), msg );
}

status_t
im_load_client_template( const char * client, BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/clients");
	path.Append( client );
		
	return im_load_template( path.Path(), msg );
}

// SAVE SETTINGS

status_t
im_save_settings( const char * path, const BMessage * settings )
{
	return im_save_attribute( path, "im_settings", settings );
}

status_t
im_save_template( const char * path, const BMessage * settings )
{
	return im_save_attribute( path, "im_template", settings );
}

status_t
im_save_protocol_settings( const char * protocol, const BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/add-ons/protocols");
	path.Append( protocol );
		
	return im_save_settings( path.Path(), msg );
}

status_t
im_save_protocol_template( const char * protocol, const BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/add-ons/protocols");
	path.Append( protocol );
		
	return im_save_template( path.Path(), msg );
}

status_t
im_save_client_settings( const char * client, const BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/clients");
	path.Append( client );
		
	return im_save_settings( path.Path(), msg );
}

status_t
im_save_client_template( const char * client, const BMessage * msg )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return B_ERROR;
		
	path.Append("im_kit/clients");
	path.Append( client );
		
	return im_save_template( path.Path(), msg );
}

// CLIENT / PROTOCOL LIST

void
im_get_file_list( const char * path, const char * msg_field, BMessage * msg )
{
	BDirectory dir(path);
	
	entry_ref ref;
	
	while ( dir.GetNextRef(&ref) == B_OK )
	{
		msg->AddString(msg_field, ref.name );
	}
}

void
im_get_protocol_list(BMessage* list)
{
	BPath path;

	if (find_directory(B_COMMON_ADDONS_DIRECTORY, &path, true, NULL) == B_OK) {
		path.Append("im_kit/protocols");
		im_get_file_list(path.Path(), "protocol", list);
	}

	if (find_directory(B_USER_ADDONS_DIRECTORY, &path, true, NULL) == B_OK) {
		path.Append("im_kit/protocols");
		im_get_file_list(path.Path(), "protocol", list);
	}
}

void
im_get_client_list( BMessage * list )
{
	BPath path;
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY,&path,true,NULL) != B_OK)
		return;
	
	path.Append("im_kit/clients");
	
	printf("%s\n", path.Path() );

	im_get_file_list( path.Path(), "client", list);
}

void
crlf2nl( const char * orig, BString & conv )
{
	conv = "";
	
	for ( int i=0; orig[i]; i++ )
	{
		if ( orig[i] != '\r' )
			conv.Append( orig[i], 1 );
	}
}

void
nl2crlf( const char * orig, BString & conv )
{
	conv = "";
	
	for ( int i=0; orig[i]; i++ )
	{
		if ( orig[i] == '\n' )
			conv.Append( '\r', 1 );
		
		conv.Append( orig[i], 1 );
	}
}


string
connection_protocol( string conn )
{
	return conn.substr(0, conn.find(":"));
}

string
connection_id( string conn )
{
	size_t colon = conn.find(":");
	
	if ( colon == string::npos )
		return "";
	
	return conn.substr(colon+1);
}
