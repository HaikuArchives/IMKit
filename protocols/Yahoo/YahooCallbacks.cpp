/*
 * sample yahoo client based on libyahoo2
 *
 * libyahoo2 is available at http://libyahoo2.sourceforge.net/
 *
 * $Revision: 1.72 $
 * $Date: 2004/01/16 05:38:46 $
 * 
 * Copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define USE_STRUCT_CALLBACKS

#include "YahooConnection.h"

#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <termios.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <assert.h>
#include "YahooConnection.h"
#include <libim/Helpers.h>

/* Get these from http://libyahoo2.sourceforge.net/ */
#include <libyahoo2/yahoo2.h>
#include <libyahoo2/yahoo2_callbacks.h>

#include "yahoo_util.h"

int fileno(FILE * stream);

#define MAX_PREF_LEN 255

//static char *local_host = NULL;

//static int do_mail_notify = 0;
//static int do_yahoo_debug = 0;
//static int ignore_system = 0;
//static int do_typing_notify = 1;
//static int accept_webcam_viewers = 1;
//static int send_webcam_images = 0;
//static int webcam_direction = YAHOO_WEBCAM_DOWNLOAD;
static time_t curTime = 0;
static time_t pingTimer = 0;
//static time_t webcamTimer = 0;
//static double webcamStart = 0;

/* id of the webcam connection (needed for uploading) */
//static int webcam_id = 0;

//static int poll_loop=1;

//static void register_callbacks();

typedef struct {
	char yahoo_id[255];
	char password[255];
	int id;
	int fd;
	int status;
	char *msg;
} yahoo_local_account;

typedef struct {
	char yahoo_id[255];
	char name[255];
	int status;
	int away;
	char *msg;
	char group[255];
} yahoo_account;

typedef struct {
	int id;
	char *label;
} yahoo_idlabel;

typedef struct {
	int id;
	char *who;
} yahoo_authorize_data;

static int connection_tags=0;

struct connect_callback_data {
	yahoo_connect_callback callback;
	void * callback_data;
	int id;
	int tag;
};

yahoo_idlabel yahoo_status_codes[] = {
	{YAHOO_STATUS_AVAILABLE, "Available"},
	{YAHOO_STATUS_BRB, "BRB"},
	{YAHOO_STATUS_BUSY, "Busy"},
	{YAHOO_STATUS_NOTATHOME, "Not Home"},
	{YAHOO_STATUS_NOTATDESK, "Not at Desk"},
	{YAHOO_STATUS_NOTINOFFICE, "Not in Office"},
	{YAHOO_STATUS_ONPHONE, "On Phone"},
	{YAHOO_STATUS_ONVACATION, "On Vacation"},
	{YAHOO_STATUS_OUTTOLUNCH, "Out to Lunch"},
	{YAHOO_STATUS_STEPPEDOUT, "Stepped Out"},
	{YAHOO_STATUS_INVISIBLE, "Invisible"},
	{YAHOO_STATUS_IDLE, "Idle"},
	{YAHOO_STATUS_OFFLINE, "Offline"},
	{YAHOO_STATUS_CUSTOM, "[Custom]"},
	{YAHOO_STATUS_NOTIFY, "Notify"},
	{0, NULL}
};

char * yahoo_status_code(enum yahoo_status s)
{
	int i;
	for(i=0; yahoo_status_codes[i].label; i++)
		if(yahoo_status_codes[i].id == s)
			return yahoo_status_codes[i].label;
	return "Unknown";
}

#define YAHOO_DEBUGLOG ext_yahoo_log

static int expired(time_t timer)
{
	if (timer && curTime >= timer)
		return 1;

	return 0;
}

static void rearm(time_t *timer, int seconds)
{
	time(timer);
	*timer += seconds;
}

FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
int gethostname(char *name, size_t len);

//static double get_time()
//{
//	struct timeval ct;
//	gettimeofday(&ct, 0);

	/* return time in milliseconds */
//	return (ct.tv_sec * 1E3 + ct.tv_usec / 1E3);
//}

static int yahoo_ping_timeout_callback( int id, time_t & pingTimer )
{
	yahoo_keepalive( id );
	rearm(&pingTimer, 600);
	return 1;
}

//static int yahoo_webcam_timeout_callback(int /*id*/)
//{
//}

//static char * get_buddy_name(char * yid)
//{
/*	YList * b;
	for (b = buddies; b; b = b->next) {
		yahoo_account * ya = (yahoo_account *)b->data;
		if(!strcmp(yid, ya->yahoo_id))
			return ya->name&&*ya->name?ya->name:ya->yahoo_id;
	}
*/
//	return yid;
//}

extern "C" int ext_yahoo_log(const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fflush(stderr);
	va_end(ap);
	return 0;
}

extern "C" void ext_yahoo_got_conf_invite(int id, const char */*me*/, const char *who, const char *room, const char *msg, YList *members)
{
}

extern "C" void ext_yahoo_conf_userdecline(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/)
{
}

extern "C" void ext_yahoo_conf_userjoin(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/)
{
}

extern "C" void ext_yahoo_conf_userleave(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/)
{
}

extern "C" void ext_yahoo_conf_message(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/, int /*utf8*/)
{
}

extern "C" void ext_yahoo_chat_cat_xml(int /*id*/, const char */*xml*/) 
{
}

extern "C" void ext_yahoo_chat_join(int /*id*/, const char */*me*/, const char */*room*/, const char * /*topic*/, YList */*members*/, int /*fd*/)
{
}

extern "C" void ext_yahoo_chat_userjoin(int /*id*/, const char */*me*/, const char */*room*/, struct yahoo_chat_member */*who*/)
{
}

extern "C" void ext_yahoo_chat_userleave(int /*id*/, const char */*me*/, const char */*room*/, const char */*who*/)
{
}

extern "C" void ext_yahoo_chat_message(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/, int /*msgtype*/, int /*utf8*/)
{
}

extern "C" void ext_yahoo_status_changed(int id, const char *who, int stat, const char *msg, int away, int idle, int /*mobile*/)
{
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbStatusChanged(who,stat,msg,away);
}

extern "C" void ext_yahoo_got_buddies(int id, YList * buds)
{
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbGotBuddies(buds);
}

extern "C" void ext_yahoo_got_ignore(int /*id*/, YList * /*igns*/)
{
}

extern "C" void ext_yahoo_got_im(int id,const char* /*me*/, const char *who, const char *msg, long tm, int stat, int utf8)
{
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbGotIM(who,msg,tm,stat,utf8);
}

extern "C" void ext_yahoo_rejected(int /*id*/, const char */*who*/, const char */*msg*/)
{
}

extern "C" void ext_yahoo_contact_added(int id, const char */*myid*/, const char *who, const char *msg)
{
}

extern "C" void ext_yahoo_typing_notify(int id, const char */*me*/, const char *who, int stat)
{
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbTypeNotify(who,stat);
}

extern "C" void ext_yahoo_game_notify(int /*id*/, const char */*me*/, const char */*who*/, int /*stat*/)
{
}

extern "C" void ext_yahoo_mail_notify(int id, const char *from, const char *subj, int cnt)
{
}

extern "C" void ext_yahoo_got_buddyicon(int id, const char */*me*/, const char *who, const char *url, int checksum)
{
	yahoo_get_url_handle(id,url,cb_yahoo_url_handle,(void*)who);
}

extern "C" void ext_yahoo_got_buddyicon_checksum(int id, const char */*me*/,const char *who, int /*checksum*/)
{
	yahoo_buddyicon_request(id,who);
}

extern "C" void ext_yahoo_got_buddyicon_request(int id, const char */*me*/, const char *who)
{
}

extern "C" void ext_yahoo_buddyicon_uploaded(int id, const char *url)
{
}

extern "C" void ext_yahoo_got_webcam_image(int /*id*/, const char */*who*/,
		const unsigned char */*image*/, unsigned int /*image_size*/, unsigned int /*real_size*/,
		unsigned int /*timestamp*/)
{
}

extern "C" void ext_yahoo_webcam_viewer(int /*id*/, const char */*who*/, int /*connect*/)
{
}

extern "C" void ext_yahoo_webcam_closed(int /*id*/, const char */*who*/, int /*reason*/)
{
}

extern "C" void ext_yahoo_webcam_data_request(int /*id*/, int /*send*/)
{
}

extern "C" void ext_yahoo_webcam_invite(int /*id*/, const char */*me*/, const char */*from*/)
{
}

extern "C" void ext_yahoo_webcam_invite_reply(int /*id*/, const char */*me*/, const char */*from*/, int /*accept*/)
{
}

extern "C" void ext_yahoo_system_message(int /*id*/, const char */*msg*/)
{
}

extern "C" void ext_yahoo_got_file(int id, const char */*me*/, const char *who, const char *url, long /*expires*/, const char *msg, const char *fname, unsigned long fesize)
{
}

extern "C" void ext_yahoo_got_identities(int /*id*/, YList * /*ids*/)
{
}

extern "C" void ext_yahoo_chat_yahoologout(int /*id*/, const char */*me*/)
{ 
}

extern "C" void ext_yahoo_chat_yahooerror(int /*id*/, const char */*me*/)
{ 
}

extern "C" void ext_yahoo_got_search_result(int /*id*/, int /*found*/, int /*start*/, int /*total*/, YList */*contacts*/)
{
}

extern "C" void ext_yahoo_got_cookies(int id)
{
	assert( gYahooConnections[id] != NULL );
	yahoo_get_yab(id);
}

extern "C" void ext_yahoo_login_response(int id, int succ, const char *url)
{
	LOG(kProtocolName, liDebug, "ext_yahoo_login_response");
	
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbLoginResponse(succ,url);
}

extern "C" void ext_yahoo_error(int id, const char *err, int fatal, int /*num*/)
{
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbYahooError(err,fatal);
}

extern "C" void ext_yahoo_got_ping(int /*id*/, const char */*errormsg*/)
{
}

fd_conn * gConnectConn=NULL;

extern "C" int ext_yahoo_add_handler(int id, int fd, yahoo_input_condition cond, void *data)
{
//	LOG(kProtocolName, liDebug, "ext_yahoo_add_handler %d", id);
	
	if ( id < 0 )
	{ // 'connect' connection
		struct fd_conn *c = (struct fd_conn*)calloc(sizeof(fd_conn), 1);
		c->tag = ++connection_tags;
		c->id = id;
		c->fd = fd;
		c->cond = cond;
		c->data = data;
		
		gConnectConn = c;
		
		return c->tag;
	}
	
	assert( gYahooConnections[id] != NULL );

	struct fd_conn *c = (struct fd_conn*)calloc(sizeof(fd_conn), 1);
	c->tag = ++connection_tags;
	c->id = id;
	c->fd = fd;
	c->cond = cond;
	c->data = data;

//	LOG(kProtocolName, liDebug, "  Add %d for %d, tag %d, cond %d", fd, id, c->tag, c->cond);
	
	gYahooConnections[id]->AddConnection( c );
	
	return c->tag;
}

extern "C" void ext_yahoo_remove_handler(int id, int tag)
{
//	LOG(kProtocolName, liDebug, "ext_yahoo_remove_handler %d", id);
	
	if ( id < 0 )
	{
		return;
	}
	
	assert( gYahooConnections[id] != NULL );
	
	YahooConnection * conn = gYahooConnections[id];
	
	for (int i=0; i < conn->CountConnections(); i++) {
		struct fd_conn *c = conn->ConnectionAt(i);
		if(c->tag == tag) {
			/* don't actually remove it, just mark it for removal */
			/* we'll remove when we start the next poll cycle */
//			LOG(kProtocolName, liDebug, "  Marking id:%d fd:%d tag:%d for removal", c->id, c->fd, c->tag);
			c->remove = 1;
			return;
		}
	}
}

extern "C" int ext_yahoo_connect(const char *host, int port)
{
	struct sockaddr_in serv_addr;
	static struct hostent *server;
	static char last_host[256];
	int servfd;
	char **p;

	if(last_host[0] || strcasecmp(last_host, host)!=0) {
		if(!(server = gethostbyname(host))) {
			LOG(kProtocolName, liDebug, "failed to look up server (%s:%d)\n%d: %s", 
						host, port,
						h_errno, strerror(h_errno));
			return -1;
		}
		strncpy(last_host, host, 255);
	}
	
	if((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		LOG(kProtocolName, liDebug, "Socket create error (%d): %s", errno, strerror(errno));
		return -1;
	}
	
	LOG(kProtocolName, liDebug, "connecting to %s:%d", host, port);
	
	for (p = server->h_addr_list; *p; p++)
	{
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr, *p, server->h_length);
		serv_addr.sin_port = htons(port);

		LOG(kProtocolName, liDebug, "trying %s", inet_ntoa(serv_addr.sin_addr));
		if(connect(servfd, (struct sockaddr *) &serv_addr, 
					sizeof(serv_addr)) == -1) {
			if(errno!=ECONNREFUSED && errno!=ETIMEDOUT && 
					errno!=ENETUNREACH) {
				break;
			}
		} else {
			LOG(kProtocolName, liDebug, "connected");
			return servfd;
		}
	}
	
	LOG(kProtocolName, liDebug, "Could not connect to %s:%d\n%d:%s", host, port, errno, 
				strerror(errno));
	close(servfd);
	return -1;
}

extern "C" int ext_yahoo_connect_async(int id, const char *host, int port, 
		yahoo_connect_callback callback, void *data)
{
	struct sockaddr_in serv_addr;
	static struct hostent *server;
	int servfd;
	struct connect_callback_data * ccd;
	int error;

	if(!(server = gethostbyname(host))) {
		errno=h_errno;
		return -1;
	}
	
	if((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, *server->h_addr_list, server->h_length);
	serv_addr.sin_port = htons(port);
	
	error = connect(servfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	
	LOG(kProtocolName, liDebug, "Trying to connect: fd:%d error:%d", servfd, error);
	if(!error) {
		LOG(kProtocolName, liDebug, "  connected!");
		callback(servfd, 0, data);
		return 0;
	} else if(error == -1 && errno == EINPROGRESS) {
		LOG(kProtocolName, liDebug, "  in progress...");
		ccd = (struct connect_callback_data*)calloc(1, sizeof(struct connect_callback_data));
		ccd->callback = callback;
		ccd->callback_data = data;
		ccd->id = id;
		
		ccd->tag = ext_yahoo_add_handler(-1, servfd, YAHOO_INPUT_WRITE, ccd);
		return ccd->tag;
	} else {
		LOG(kProtocolName, liDebug, "  error!");
		if(error == -1) {
			LOG(kProtocolName, liDebug, "%s", strerror(errno));
		}
		close(servfd);
		return -1;
	}
}

/*************************************
 * Callback handling code starts here
 */
 
void cb_yahoo_url_handle(int id, int fd, int error, const char */*filename*/, unsigned long size, void *data) 
{
	const char *who = (const char*)data;
	char byte;
	BString buff;
	unsigned long count = 0;
	while (count <= size) 
	{
		read(fd,&byte,1);
		count++;
		buff << byte;
	}
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbGotBuddyIcon(who,size,buff.String());
}

static void connect_complete(void *data, int source, yahoo_input_condition /*condition*/)
{
	struct connect_callback_data *ccd = (struct connect_callback_data *)data;
	int error, err_size = sizeof(error);

//	ext_yahoo_remove_handler(0, ccd->tag);
#ifdef __HAIKU__
	getsockopt(source, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&err_size);
#else
	getsockopt(source, SOL_SOCKET, SO_ERROR, &error, (int *)&err_size);
#endif
	
	if(error) {
		close(source);
		source = -1;
	}
	
	LOG(kProtocolName, liDebug, "Connected fd: %d, error: %d", source, error);
	
	ccd->callback(source, error, ccd->callback_data);
	FREE(ccd);
}

void yahoo_callback(struct fd_conn *c, yahoo_input_condition cond)
{
	int ret=1;
	char buff[1024]={0};
	
	if(c->id < 0) {
		connect_complete(c->data, c->fd, cond);
		LOG(kProtocolName, liDebug, "calling connect_complete()");
	} else {
		if(cond & YAHOO_INPUT_READ) {
			ret = yahoo_read_ready(c->id, c->fd, c->data);
//			LOG(kProtocolName, liDebug, "Data read fd: %d, tag: %d, cond: %d", c->fd, c->tag, c->cond);
		}
		if(ret>0 && cond & YAHOO_INPUT_WRITE) {
			ret = yahoo_write_ready(c->id, c->fd, c->data);
//			LOG(kProtocolName, liDebug, "Data written fd: %d, tag: %d, cond: %d", c->fd, c->tag, c->cond);
		}

		if(ret == -1)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error (%d): %s", errno, strerror(errno));
		else if(ret == 0)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error: Server closed socket");
		
		if(buff[0])
			LOG(kProtocolName, liDebug, "Error: %s", buff);
	}
}

/*
 * Callback handling code ends here
 ***********************************/

int32
yahoo_io_thread( void * _data )
{
	YahooConnection * conn = (YahooConnection*)_data;
	
#if 0
	conn->fID = yahoo_init_with_attributes(
		conn->fYahooID, conn->fPassword,
		//"local_host", local_host,
		"pager_port", 23,
		NULL
	);
#else
	conn->fID = yahoo_init(
		conn->fYahooID, conn->fPassword
	);
#endif
	
	LOG(kProtocolName, liDebug, "yahoo_io_thread: id: %s, pass: %s", conn->fYahooID, conn->fPassword );
	
	gYahooConnections[conn->fID] = conn;
	
	yahoo_login( conn->fID, YAHOO_STATUS_AVAILABLE );

	int lfd=0;
	
	fd_set inp, outp;
	struct timeval tv;
	
	LOG(kProtocolName, liDebug, "yahoo_io_thread: Starting loop");
	
	while( conn->IsAlive() ) {
		FD_ZERO(&inp);
		FD_ZERO(&outp);
		tv.tv_sec=1;
		tv.tv_usec=0;
		lfd=0;
		
		for(int i=0; i<conn->CountConnections(); i++) {
			struct fd_conn *c = conn->ConnectionAt(i);
			
			if(c->remove) {
//				LOG(kProtocolName, liDebug, "yahoo_io_thread: Removing id:%d fd:%d", c->id, c->fd);
				conn->RemoveConnection(c);
				free(c);
			} else {
				if(c->cond & YAHOO_INPUT_READ) {
					FD_SET(c->fd, &inp);
				}
				if(c->cond & YAHOO_INPUT_WRITE) {
					FD_SET(c->fd, &outp);
				}
				if(lfd < c->fd)
					lfd = c->fd;
			}
		}
		
		select(lfd + 1, &inp, &outp, NULL, &tv);
		time(&curTime);
		
		for(int i=0; i<conn->CountConnections(); i++) {
			struct fd_conn *c = conn->ConnectionAt(i);
			if(c->remove)
				continue;
			if(FD_ISSET(c->fd, &inp)) {
//				LOG(kProtocolName, liDebug, "yahoo_io_thread: data to read");
				yahoo_callback(c, YAHOO_INPUT_READ);
				FD_CLR(c->fd, &inp);
			}
			if(FD_ISSET(c->fd, &outp)) {
				//LOG(kProtocolName, liDebug, "yahoo_io_thread: data to write");
				yahoo_callback(c, YAHOO_INPUT_WRITE);
				FD_CLR(c->fd, &outp);
			}
		}
		
		if(expired(pingTimer))
			yahoo_ping_timeout_callback(conn->fID, pingTimer);
	}
	LOG(kProtocolName, liDebug, "yahoo_io_thread: Exited loop");
	
	while( conn->CountConnections() > 0 ) {
		struct fd_conn * c = conn->ConnectionAt(0);
		conn->RemoveConnection(c);
		close(c->fd);
		FREE(c);
	}
	
	return 0;
}

extern "C" void register_callbacks()
{
	static struct yahoo_callbacks yc;
	
	yc.ext_yahoo_login_response = ext_yahoo_login_response;
	yc.ext_yahoo_got_buddies = ext_yahoo_got_buddies;
	yc.ext_yahoo_got_ignore = ext_yahoo_got_ignore;
	yc.ext_yahoo_got_identities = ext_yahoo_got_identities;
	yc.ext_yahoo_got_cookies = ext_yahoo_got_cookies;
	yc.ext_yahoo_status_changed = ext_yahoo_status_changed;
	yc.ext_yahoo_got_im = ext_yahoo_got_im;
	yc.ext_yahoo_got_conf_invite = ext_yahoo_got_conf_invite;
	yc.ext_yahoo_conf_userdecline = ext_yahoo_conf_userdecline;
	yc.ext_yahoo_conf_userjoin = ext_yahoo_conf_userjoin;
	yc.ext_yahoo_conf_userleave = ext_yahoo_conf_userleave;
	yc.ext_yahoo_conf_message = ext_yahoo_conf_message;
	yc.ext_yahoo_chat_cat_xml = ext_yahoo_chat_cat_xml;
	yc.ext_yahoo_chat_join = ext_yahoo_chat_join;
	yc.ext_yahoo_chat_userjoin = ext_yahoo_chat_userjoin;
	yc.ext_yahoo_chat_userleave = ext_yahoo_chat_userleave;
	yc.ext_yahoo_chat_message = ext_yahoo_chat_message;
	yc.ext_yahoo_chat_yahoologout = ext_yahoo_chat_yahoologout;
	yc.ext_yahoo_chat_yahooerror = ext_yahoo_chat_yahooerror;
	yc.ext_yahoo_got_webcam_image = ext_yahoo_got_webcam_image;
	yc.ext_yahoo_webcam_invite = ext_yahoo_webcam_invite;
	yc.ext_yahoo_webcam_invite_reply = ext_yahoo_webcam_invite_reply;
	yc.ext_yahoo_webcam_closed = ext_yahoo_webcam_closed;
	yc.ext_yahoo_webcam_viewer = ext_yahoo_webcam_viewer;
	yc.ext_yahoo_webcam_data_request = ext_yahoo_webcam_data_request;
	yc.ext_yahoo_got_file = ext_yahoo_got_file;
	yc.ext_yahoo_contact_added = ext_yahoo_contact_added;
	yc.ext_yahoo_rejected = ext_yahoo_rejected;
	yc.ext_yahoo_typing_notify = ext_yahoo_typing_notify;
	yc.ext_yahoo_game_notify = ext_yahoo_game_notify;
	yc.ext_yahoo_mail_notify = ext_yahoo_mail_notify;
	yc.ext_yahoo_got_search_result = ext_yahoo_got_search_result;
	yc.ext_yahoo_system_message = ext_yahoo_system_message;
	yc.ext_yahoo_error = ext_yahoo_error;
	yc.ext_yahoo_log = ext_yahoo_log;
	yc.ext_yahoo_add_handler = ext_yahoo_add_handler;
	yc.ext_yahoo_remove_handler = ext_yahoo_remove_handler;
	yc.ext_yahoo_connect = ext_yahoo_connect;
	yc.ext_yahoo_connect_async = ext_yahoo_connect_async;
	yc.ext_yahoo_got_buddyicon = ext_yahoo_got_buddyicon;
	yc.ext_yahoo_got_buddyicon_checksum = ext_yahoo_got_buddyicon_checksum;
	yc.ext_yahoo_got_buddyicon_request = ext_yahoo_got_buddyicon_request;
	yc.ext_yahoo_buddyicon_uploaded = ext_yahoo_buddyicon_uploaded;
	yc.ext_yahoo_got_ping = ext_yahoo_got_ping;
	
	yahoo_register_callbacks(&yc);
}
