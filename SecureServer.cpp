#include "SecureServer.h"

void SecureServer::verifyCertificates(SSL_CTX *ssl_context, 
    const char *certificateKeyFile, 
    const char *certificateFile)
{
    if (SSL_CTX_use_certificate_file(ssl_context, certificateFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if (SSL_CTX_use_PrivateKey_file(ssl_context, certificateKeyFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if (!SSL_CTX_check_private_key(ssl_context))
    {
        fprintf(stderr, "Error in validating certificates\n");
        abort();
    }
}

SSL_CTX *SecureServer::initializeSSLContext(void)
{
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *sslContext;

    // initialize and configure OpenSSL parameters
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();       /* load all error messages */
    sslContext = SSL_CTX_new(method);      /* create new context from method */
    if (sslContext == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return sslContext;
}

void SecureServer::startListeningServer()
{
    int socketOptions = 1;

    FD_SET(STDIN, &globalFieldDescriptor);
    struct sockaddr_in socketAddress;

    socketAddress.sin_port = htons(serverListeningPort);
    
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    

    if ((listeningSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Error in listening socket");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(listeningSocketDescriptor, 
        SOL_SOCKET, 
        SO_REUSEADDR, 
        (char *)&socketOptions, sizeof(socketOptions)) < 0)
    {
        perror("wrong socker options");
        exit(EXIT_FAILURE);
    }
    
    if (::bind(listeningSocketDescriptor, 
        (struct sockaddr *)&socketAddress,
         sizeof(socketAddress)) < 0)
    {
        exit(EXIT_FAILURE);
    }
    printf("Listening started on port: %d \n", serverListeningPort);
    if (listen(listeningSocketDescriptor, 5))
    {
        perror("Error in listening for new client connections");
        exit(EXIT_FAILURE);
    }
    FD_SET(listeningSocketDescriptor, &globalFieldDescriptor);
    //m_nMaxFd = listeningSocketDescriptor;
}

void SecureServer::listeningEvents()
{
    SSL_library_init();
    SSL_CTX *sslContext = initializeSSLContext();
    
    char buffer[1024];
    verifyCertificates(sslContext, "sslcertificate.pem", "sslcertificate.pem");
    startListeningServer();
    while (1)
    {
        SSL *sslInstance;
        struct sockaddr_in socketAddress;
        socklen_t length = sizeof(socketAddress);

        int clientThread = accept(listeningSocketDescriptor, (struct sockaddr *)&socketAddress, &length);
        printf("New Client Request Received: %s %d\n", 
            inet_ntoa(socketAddress.sin_addr), ntohs(socketAddress.sin_port));
        sslInstance = SSL_new(sslContext);
        SSL_set_fd(sslInstance, clientThread);
        sslHandshake(sslInstance);
    }
    close(listeningSocketDescriptor); /* close server socket */
    SSL_CTX_free(sslContext);  /* release context */
}

void SecureServer::sslHandshake(SSL *sslInstance)
{
    const char *informationMessage = "<html><body><pre>%s</pre></body></html>\n\n";
    char responseBuffer[1024];
    char buf2[1024];
    int bytesBuffer, socketDescriptor;
    
    if (SSL_accept(sslInstance) == 0) {
        ERR_print_errors_fp(stderr);
    }
    else
    {
        bytesBuffer = SSL_read(sslInstance, buf2, sizeof(buf2));
        if (bytesBuffer > 0)
        {
            buf2[bytesBuffer] = 0;
            sprintf(responseBuffer, informationMessage, buf2);
            SSL_write(sslInstance, responseBuffer, strlen(responseBuffer));
        }
        else {
            ERR_print_errors_fp(stderr);
        }
    }
    socketDescriptor = SSL_get_fd(sslInstance);
    // release all the occupied OpenSSL and system resources
    SSL_free(sslInstance);
    close(socketDescriptor);
}


