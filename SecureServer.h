#ifndef SERVER_H
#define SERVER_H

#include "Common.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <cstdio>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <netinet/in.h>

using namespace std;

class SecureServer
{
public:
	SecureServer(int portNumber) {
		cout << "**** Secure Chat Server Started****" << endl;

		this->serverListeningPort = portNumber;
		
		FD_ZERO(&globalFieldDescriptor);
		FD_ZERO(&readFieldDescriptor);

		// starting event handler
		listeningEvents();
	}
	
	~SecureServer() {

	}

private:
	fd_set readFieldDescriptor;
	fd_set globalFieldDescriptor;
	// listening port of secure chat server
	int serverListeningPort;
	int listeningSocketDescriptor;
private:
	void verifyCertificates(SSL_CTX *ssl_context, const char *certificateKeyFile, const char *certificateFile);
	void sslHandshake(SSL *ssl);
	SSL_CTX *initializeSSLContext(void);
private:
	void listeningEvents();
	void startListeningServer();
	void commandShell();
	void displayUsage();
};

#endif

