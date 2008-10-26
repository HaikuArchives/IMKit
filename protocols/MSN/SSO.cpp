#include "SSO.h"

#include "MSN.h"
#include "HTTPFormatter.h"
#include "SSOHandler.h"
#include "common/BufferWriter.h"
#include "common/Base64.h"

#ifdef BONE_BUILD
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
#else
	#include <net/socket.h>
	#include <net/netdb.h>
#endif
#include <sys/select.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/des.h>

//#pragma mark Helper Functions

status_t ExtractXMLChunk(const BString &xml, const BString &element, const BString &childElement,
	const BString &childElementValue, BString &value) {
	
	status_t result = B_ERROR;
	int32 start = B_ERROR;
	int32 offset = 0;
	BString startElement = "<";
	BString endElement = "</";
	BString child = "<";
	
	startElement << element << ">";
	endElement << element << ">";
	child << childElement << ">" << childElementValue << "</" << childElement << ">";

	while ((start = xml.IFindFirst(startElement, offset)) != B_ERROR) {
		int32 end = xml.IFindFirst(endElement, start);
		value = "";
		
		if (end != B_ERROR) {		
			xml.CopyInto(value, start, end - start);
			
			if (value.IFindFirst(child) != B_ERROR) {
				result = B_OK;
				return result;
			}
		};
		
		offset = start + startElement.Length();
	};
	
	return result;
};

status_t ExtractXMLNode(const BString &xml, const BString &element, const BString &attr,
	const BString &attrValue, BString &value) {
	
	status_t result = B_ERROR;
	BString temp = "<";
	temp << element;
	if ((attr.Length() > 0) && (attrValue.Length() > 0)) {
		temp << " " << attr << "=\""  << attrValue << "\"";
	};
	temp << ">";

	int32 open = xml.IFindFirst(temp);
	if (open != B_ERROR) {
		int start = open + temp.Length();
		temp = "";
		temp << "</" << element << ">";
		int32 end = xml.IFindFirst(temp, start);
		if (end != B_ERROR) {
			xml.CopyInto(value, start, end - start);
			result = B_OK;
		};
	};
	
	return result;
};

int32 DeriveKey(const void *key, int32 keyLen, const uchar *magic, int32 magicLen, uchar **result) {
	uchar hash1[EVP_MAX_MD_SIZE];
	uchar hash2[EVP_MAX_MD_SIZE];
	uchar hash3[EVP_MAX_MD_SIZE];
	uchar hash4[EVP_MAX_MD_SIZE];
	unsigned int hash1Len = 0;
	unsigned int hash2Len = 0;
	unsigned int hash3Len = 0;
	unsigned int hash4Len = 0;
	int32 length = B_ERROR;
	BMallocIO temp;
	
	// HMAC-SHA1(magic)
	HMAC(EVP_sha1(), key, keyLen, magic, magicLen, hash1, &hash1Len);

	// Key 2 is HMAC-SHA1(HMAC-SHA1(magic) + magic)
	temp.Write(hash1, hash1Len);
	temp.Write(magic, magicLen);
	HMAC(EVP_sha1(), key, keyLen, (uchar *)temp.Buffer(), temp.BufferLength(), hash2, &hash2Len);

	// HMAC-SHA1(HMAC-SHA1(magic))
	HMAC(EVP_sha1(), key, keyLen, hash1, hash1Len, hash3, &hash3Len);
			
	// Clear the BMallocIO and reset the position to 0
	temp.SetSize(0);
	temp.Seek(0, SEEK_SET);

	// Key 4 is HMAC-SHA1(HMAC-SHA1(HMAC-SHA1(magic)) + magic)
	temp.Write(hash3, hash3Len);
	temp.Write(magic, magicLen);
	HMAC(EVP_sha1(), key, keyLen, (uchar *)temp.Buffer(), temp.BufferLength(), hash4, &hash4Len);

	// The key is Hash2 followed by the first four bytes of Hash4
	length = hash2Len + 4;
	*result = (uchar *)calloc(length, sizeof(uchar));

	memcpy(*result, hash2, hash2Len);
	memcpy(*result + hash2Len, hash4, 4);
	
	return length;
};

//#pragma mark Constructor

SSO::SSO(SSOHandler *handler, const char *passport, const char *password, const char *URI,
	const char *nonce)
	: fHandler(handler),
	fPassport(passport),
	fPassword(password),
	fURI(URI),
	fNonce(nonce) {
	
	fMethods["MBI_KEY_OLD"] = &SSO::MBIKeyOld;
};

SSO::~SSO(void) {
};

//#pragma mark Public

status_t SSO::Response(BString &token, BString &response) {
	status_t result = B_ERROR;
	SSOMethod method;
	ssomethod_t::iterator sIt = fMethods.find(fURI);
	if (sIt != fMethods.end()) {
		method = sIt->second;

		BString soap = "<Envelope xmlns=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"xmlns:wsse=\"http://schemas.xmlsoap.org/ws/2003/06/secext\" "
			"xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\" "
			"xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2002/12/policy\" "
			"xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" "
			"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/03/addressing\" "
			"xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2004/04/sc\" "
			"xmlns:wst=\"http://schemas.xmlsoap.org/ws/2004/04/trust\">"
			"<Header>"
       		"<ps:AuthInfo "
			"xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" "
           	"Id=\"PPAuthInfo\">"
           	"<ps:HostingApp>{7108E71A-9926-4FCB-BCC9-9A9D3F32E423}</ps:HostingApp>"
           	"<ps:BinaryVersion>4</ps:BinaryVersion>"
           	"<ps:UIVersion>1</ps:UIVersion>"
           	"<ps:Cookies></ps:Cookies>"
           	"<ps:RequestParams>AQAAAAIAAABsYwQAAAAxMDMz</ps:RequestParams>"
       		"</ps:AuthInfo>"
       		"<wsse:Security>"
           	"<wsse:UsernameToken Id=\"user\">"
			"<wsse:Username>";
		soap << fPassport << "</wsse:Username>"
			"<wsse:Password>" << fPassword << "</wsse:Password>"
			"</wsse:UsernameToken>"
			"</wsse:Security>"
			"</Header>"
			"<Body>"
			"<ps:RequestMultipleSecurityTokens "
			"xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" "
			"Id=\"RSTS\">"
			"<wst:RequestSecurityToken Id=\"RST0\">"
			"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
			"<wsp:AppliesTo>"
			"<wsa:EndpointReference>"
			"<wsa:Address>http://Passport.NET/tb</wsa:Address>"
			"</wsa:EndpointReference>"
			"</wsp:AppliesTo>"
			"</wst:RequestSecurityToken>"
			"<wst:RequestSecurityToken Id=\"RST0\">"
			"<wst:RequestType>http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue</wst:RequestType>"
			"<wsp:AppliesTo>"
			"<wsa:EndpointReference>"
			// Domain
			"<wsa:Address>messengerclear.live.com</wsa:Address>"
			"</wsa:EndpointReference>"
			"</wsp:AppliesTo>";
		soap << "<wsse:PolicyReference URI=\"" << fURI << "\"></wsse:PolicyReference>"
			"</wst:RequestSecurityToken>"
			"</ps:RequestMultipleSecurityTokens>"
			"</Body>"
			"</Envelope>";
		
		HTTPFormatter *send = new HTTPFormatter("login.live.com", "/RST.srf");
		send->RequestType("POST");
		send->AddHeader("Content-Type", "application/soap+xml; charset=\"utf-8\"");
		send->SetContent(soap.String(), soap.Length());
		
		if (fHandler) fHandler->SSORequestingTicket();
	
		HTTPFormatter *recv = NULL;
		
		int32 sslBytes = SSLSend("login.live.com", send, &recv);
		if ((sslBytes >= 0) && (recv != NULL)){
			BString contents(recv->Content(), recv->ContentLength());
			
			BString id = "Compact1";
			BString element  = "wsse:BinarySecurityToken";
			if (fURI[0] == '?') id = "PPToken1";

			BString secret = "";
			BString tempXML = "";

			ExtractXMLChunk(contents, BString("wst:RequestSecurityTokenResponse"),
				BString("wst:TokenType"), BString("urn:passport:compact"), tempXML);
			ExtractXMLNode(tempXML, BString("wst:BinarySecret"), BString(""), BString(""), secret);
			ExtractXMLNode(tempXML, element, BString("Id"),  id, token);
			
			char *trail = "&amp;p=";
			token.Remove(token.Length() - strlen(trail), strlen(trail));
			
			if (fHandler) fHandler->SSOGeneratingResponse();

			result = (this->*method)(secret, fNonce, response);
		} else {
			if (fHandler) fHandler->SSOError("Unable to request ticket");
		};
	
		delete send;
		delete recv;
	};
	
	return result;
};

//#pragma mark Private

#define CHK_NULL(x) if ((x)==NULL) return -1
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); return -1; }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); return -1; }

status_t SSO::SSLSend(const char *host, HTTPFormatter *send, HTTPFormatter **recv) {
	int err = B_OK;
	int sd;
	struct sockaddr_in sa;
	struct hostent *hp;
	SSL_CTX *ctx;
	SSL *ssl;
	char buffer[1024];
	SSL_METHOD *meth;

	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	meth = SSLv23_client_method();
	ctx = SSL_CTX_new (meth);
	CHK_NULL(ctx);
	SSL_CTX_set_options(ctx, SSL_OP_ALL);

	/* ----------------------------------------------- */
	/* Create a socket and connect to server using normal socket calls. */
	sd = socket (AF_INET, SOCK_STREAM, 0);
	CHK_ERR(sd, "socket");

	// clear sa
	memset (&sa, '\0', sizeof(sa));
	
	// get address
	if ((hp= gethostbyname(host)) == NULL) { 
		sa.sin_addr.s_addr = inet_addr (host);   /* Server IP */
	} else {
		memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length); /* set address */
	};
	
	sa.sin_family = AF_INET;
	sa.sin_port = htons(443);    /* Server Port number */
	
	err = connect(sd, (struct sockaddr*) &sa, sizeof(sa));
	CHK_ERR(err, "connect");
	
	/* ----------------------------------------------- */
	/* Now we have TCP conncetion. Start SSL negotiation. */
  
	ssl = SSL_new (ctx);                         CHK_NULL(ssl);    
	if (SSL_set_fd(ssl, sd) == 0) {
		LOG(kProtocolName, liDebug, "C %lX: SSL Error setting fd", this);
		return -1;
	};
	
    SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);
	SSL_set_connect_state(ssl);
	
	err = SSL_connect (ssl);
	CHK_SSL(err);
	
	/* --------------------------------------------------- */
	/* DATA EXCHANGE - Send a message and receive a reply. */

	err = SSL_write(ssl, send->Flatten(), send->Length());
	CHK_SSL(err);
	
	if (err <= 0) {
		LOG(kProtocolName, liDebug, "C %lX: SSL Error writing. Err: %ld", this, SSL_get_error(ssl, err));
	};
	
	BString data;
	int received = 0;
	while (err > 0) {
		err = SSL_read(ssl, buffer, sizeof(buffer));
		CHK_SSL(err);
		if (err > 0) {
			received += err;
			data.Append(buffer, err);
			memset(buffer, 0, sizeof(buffer));
		};
	};

	*recv = new HTTPFormatter(data.String(), data.Length());
	LOG(kProtocolName, liDebug, "C %lX: Got %d chars", this, received);
	SSL_shutdown (ssl);  /* send SSL/TLS close_notify */
	
	/* Clean up. */
	close (sd);
	SSL_free (ssl);
	SSL_CTX_free (ctx);
	
	return received;
};

status_t SSO::MBIKeyOld(BString key, BString nonce, BString &response) {
	const char *kSessionHash = "WS-SecureConversationSESSION KEY HASH";
	const char *kSessionEnc = "WS-SecureConversationSESSION KEY ENCRYPTION";
	const uint32 kSessionHashLen = strlen(kSessionHash);
	const uint32 kSessionEncLen = strlen(kSessionEnc);
	const uint8 kNoncePadding = 8;

	uchar *key1 = NULL;
	uchar *key2 = NULL;
	uchar *key3 = NULL;

	// Obtain the three keys for the 3DES encryption
	int32 key1Len = (int32)Base64Decode(key.String(), key.Length(), &key1);
	int32 key2Len = DeriveKey(key1, key1Len, (uchar *)kSessionHash, kSessionHashLen, &key2);
	DeriveKey(key1, key1Len, (uchar *)kSessionEnc, kSessionEncLen, &key3);

	uchar hash[EVP_MAX_MD_SIZE];
	unsigned int hashLen = 0;
	
	// Key 2 is used as the key to the HMAC-SHA() of the Nonce
	HMAC(EVP_sha1(), key2, key2Len, (uchar *)nonce.String(), nonce.Length(), hash, &hashLen);

	// The nonce is padded out by 8 bytes of 0x08
	int32 padNonceLen = nonce.Length() + kNoncePadding;
	uchar *padNonce = (uchar *)calloc(padNonceLen, sizeof(uchar));
	memset(padNonce, 0x08, padNonceLen);
	memcpy(padNonce, nonce.String(), nonce.Length());
	
	const uint32 kKeyLen = 8;
	uchar cbc_key1[kKeyLen];
	uchar cbc_key2[kKeyLen];
	uchar cbc_key3[kKeyLen];
	
	// The 3DES CBC key is Key3
	memcpy(cbc_key1, key3 + (kKeyLen * 0), sizeof(cbc_key1));
	memcpy(cbc_key2, key3 + (kKeyLen * 1), sizeof(cbc_key2));
	memcpy(cbc_key3, key3 + (kKeyLen * 2), sizeof(cbc_key3));

	des_key_schedule ks1;
	des_key_schedule ks2;
	des_key_schedule ks3;
	
	// Create 3DES CBC keys
	DES_set_key(&cbc_key1, &ks1);
	DES_set_key(&cbc_key2, &ks2);
	DES_set_key(&cbc_key3, &ks3);
	
	// Setup the IV
	uchar iv[8];
	for (int8 i = 0; i < 8; i++) iv[i] = rand() * 255;
	des_cblock iv3;
	memcpy(iv3, iv, sizeof(iv));

	// Create a buffer used by the 3DES process
	uchar *buffer = (uchar *)calloc(1024, sizeof(uchar));

	// 3DES the nonce
	DES_ede3_cbc_encrypt(padNonce, buffer, padNonceLen, &ks1, &ks2, &ks3, &iv3, DES_ENCRYPT);

	// Output the results
	BufferWriter *result = new BufferWriter(B_SWAP_HOST_TO_LENDIAN);
	result->WriteInt32(0x0000001C);			// Size of header (28)
	result->WriteInt32(0x00000001);			// CBC crypt
	result->WriteInt32(0x00006603);			// Triple DES
	result->WriteInt32(0x00008004);			// SHA1
	result->WriteInt32(sizeof(iv));			// Length of IV
	result->WriteInt32(hashLen);			// Length of hash
	result->WriteInt32(72);					// Length of cipher
	result->WriteData(iv, sizeof(iv));		// IV
	result->WriteData(hash, hashLen);		// Hash
	result->WriteData(buffer, 72);			// Enc
	
	char *base64Result = Base64Encode((const char *)result->Buffer(), (int32)result->Offset());
	response = base64Result;

	free(key1);
	free(key2);
	free(key3);
	free(padNonce);
	free(buffer);
	free(base64Result);
	
	return B_OK;
};
