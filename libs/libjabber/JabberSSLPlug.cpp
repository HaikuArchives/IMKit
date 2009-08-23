#include "JabberSSLPlug.h"

#include "JabberHandler.h"

#define LOG(X) printf X;

#define msnmsgPing 'ping'

JabberSSLPlug::JabberSSLPlug(BString forceserver, int32 port){

	bio = NULL;
	ctx = NULL;
	
	ffServer = forceserver;
	ffPort = port;

	Run();
	
	fKeepAliveRunner = new BMessageRunner(BMessenger(NULL, (BLooper *)this),
		new BMessage(msnmsgPing), 60000000, -1);

	SSL_library_init();
}


JabberSSLPlug::~JabberSSLPlug(){

		if ( fKeepAliveRunner)
			delete fKeepAliveRunner;
		if(bio != NULL & ctx !=NULL) StopConnection();
}

void
JabberSSLPlug::MessageReceived(BMessage* msg){

		if(msg->what == msnmsgPing){
			printf("\nPING!\n");
			Send(" ");
		}
		else
			BLooper::MessageReceived(msg);

}
int
JabberSSLPlug::StartConnection(BString fServer, int32 fPort,void* cookie){
	
	BString fHost;
	
	if(ffServer!="")	
		fHost << ffServer << ":" << ffPort;
	else
		fHost << fServer << ":" << fPort;
	
	LOG(("StartConnection to %s\n",fHost.String()));

	SSL * ssl;
	int result = 0;

	 /* Set up the library */

    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* Set up the SSL context */

     ctx = SSL_CTX_new(SSLv23_client_method());

	 bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */
	
	
    
    BIO_set_conn_hostname(bio, fHost.String());

    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        result = -1;
    }
    
		
	fCookie = cookie;

	fReceiverThread = spawn_thread (ReceiveData, "opensll receiver", B_LOW_PRIORITY, this); 
		
	if (fReceiverThread != B_ERROR) 
	   	resume_thread(fReceiverThread); 
	else 
	{
		printf("failed to resume the thread!\n");
		return -1; //FIX! returning..
	}
	
	LOG(("DONE: StartConnection to %s\n",fHost.String()));
	return result;	
}


int32
JabberSSLPlug::ReceiveData(void * pHandler){
	
	char data[1024];
	int length = 0;
	JabberSSLPlug * plug = reinterpret_cast<JabberSSLPlug * >(pHandler);
	
	while (true) 
	{
				
		if ((length = (int)BIO_read(plug->bio, data, 1023) ) > 0) 
		{
			
			data[length] = 0;
			
			#ifdef STDOUT
				printf("\nSSLPlug<< %s\n", data);
			#endif
		} 
		
		else {
		
			if(!BIO_should_retry(plug->bio)) {			
				
				//uhm really and error!
				#ifdef STDOUT
					//int ret = SSL_get_error(plug->bio);
					printf("\nSSLPlug<< not should retry! err ");
					ERR_print_errors(plug->bio);
				#endif	
				return 0;			
			}
			else 
			{
				BString space(" ");
				plug->Send(space); 
			}
		}
			
		plug->ReceivedData(data,length);
	}

	return 0;
}

void
JabberSSLPlug::ReceivedData(const char* data,int32 len){
	JabberHandler * handler = reinterpret_cast<JabberHandler * >(fCookie);
	if(handler)
		handler->ReceivedData(data,len);
}

int
JabberSSLPlug::Send(const BString & xml){
		
		#ifdef STDOUT
			printf("\nSSLPlug>> %s\n", xml.String());
		#endif
		
		return BIO_write(bio, xml.String(), xml.Length());
}

int		
JabberSSLPlug::StopConnection(){
	
	//Thread Killing!
	suspend_thread(fReceiverThread);
	
	if(fReceiverThread)	kill_thread(fReceiverThread);
	
	fReceiverThread=0;
	
	BIO_free_all(bio);
    SSL_CTX_free(ctx);

	bio = NULL;
	ctx = NULL;

	return 0;
}


//--

