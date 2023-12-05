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
private:
	// listening port of secure chat server
	int serverListeningPort;
	int m_nListenSd;
	int m_nMaxFd;
	fd_set m_masterSet;
	fd_set m_readSet;

public:
	// Constructor and destructor
	SecureServer(int port);
	~SecureServer();

private:
	void startListener();
	void eventHandler();
	void commandShell();
	void displayUsage();

	SSL_CTX *InitServerCTX(void);
	void LoadCertificates(SSL_CTX *ctx, const char *CertFile, const char *KeyFile);
	void ShowCerts(SSL *ssl);
	void Servlet(SSL *ssl);
};

#endif

