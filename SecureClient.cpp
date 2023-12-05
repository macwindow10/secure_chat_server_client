
#include "Common.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

using namespace std;

class SecureClient {
private:
    fd_set globalFieldDescriptor;
	fd_set readFieldDescriptor;
private:
	char serverIPAddress[32];
	int listeningSocketDescriotor;
    int clientListeningPort;
    int clientSocketDescriptor;
	bool isClientRegistered;

	int serverSocketDescriptor;
	int m_nMaxFd;  
	
public:
    SecureClient(int portNumber) {
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        this->clientListeningPort = portNumber;
        this->isClientRegistered = false;
        this->clientSocketDescriptor = -1;
        FD_ZERO(&globalFieldDescriptor);
        FD_ZERO(&readFieldDescriptor);

        listeningEvents();
    }

    ~SecureClient() {
        
    }

private:
    void listeningEvents() {

        char buffer[1024];
        
        startListener();

        for (;;)
        {
            // copy the global field descriptor
            readFieldDescriptor = globalFieldDescriptor;
            if (select(m_nMaxFd + 1, &readFieldDescriptor, NULL, NULL, NULL) == -1)
            {
                perror("Error in connection");
                exit(EXIT_FAILURE);
            }
            
            int bytesRead;
            char recvBuff[1024];
            for (int i = 0; i <= m_nMaxFd; i++)
            {
                if (FD_ISSET(i, &readFieldDescriptor))
                {
                    if (i == 0)
                    {
                        processCommands();
                    }
                    else if (i == listeningSocketDescriotor)
                    {
                        // new connection
                        if (isClientRegistered)
                        {
                            cout << "New Connection" << endl;
                            processNewConnection();
                        }
                    }
                }
                else
                {
                    // cout << "data from connected client" << endl;
                    //  handle data from connected clientfter registration is successful
                    if (isClientRegistered)
                    {
                        char recvBuff[1024] = {0};
                        int bytesRead;
                        if ((bytesRead = recv(i, recvBuff, sizeof(recvBuff), 0)) > 0)
                        {
                            recvBuff[strlen(recvBuff)] = 0;
                            cout << "Remote Msg: " << recvBuff << endl;
                        }
                    }
                }
            }
        }
    }

    void show_help_messages() {
        printf("Invalid command");
    }
	
    void startListener() {
        int sd;
        struct sockaddr_in m_cliListenAddr;

        FD_SET(STDIN, &globalFieldDescriptor);

        // populate server address structure
        m_cliListenAddr.sin_family = AF_INET;
        m_cliListenAddr.sin_port = htons(clientListeningPort);
        m_cliListenAddr.sin_addr.s_addr = INADDR_ANY;
        // create a TCP listening socket
        if ((listeningSocketDescriotor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        // reuse the socket in case of crash
        int optval = 1;
        if (setsockopt(listeningSocketDescriotor, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0)
        {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }
        
        if (::bind(listeningSocketDescriotor, (struct sockaddr *)&m_cliListenAddr, sizeof(m_cliListenAddr)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        printf("Client is now listening on port: %d \n", clientListeningPort);
        if (listen(listeningSocketDescriotor, 3))
        {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }
        FD_SET(listeningSocketDescriotor, &globalFieldDescriptor);
        m_nMaxFd = listeningSocketDescriotor;
    }
	
	void processCommands() {
        int nArgs, connId;
        char arg1[32], arg2[32], arg3[32];
        char *commandLine = NULL;
        size_t size;
        ssize_t linelen = getline(&commandLine, &size, stdin);
        commandLine[linelen - 1] = '\0';
        if (strlen(commandLine) == 0) {
            return;
        }            

        char *argLine = (char *)malloc(strlen(commandLine));
        strcpy(argLine, commandLine);
        nArgs = getArgumentsCount(argLine, " ");

        CommandID command = getCommand(strtok(commandLine, " "));
        switch (command)
        {
        case COMMAND_REGISTER:
            if (nArgs != 3)
            {
                show_help_messages();
                break;
            }

            if (isClientRegistered)
            {
                cout << "This client is already registered" << endl;
                break;
            }

            strncpy(arg1, strtok(NULL, " "), sizeof(arg1) - 1);
            strncpy(arg2, strtok(NULL, " "), sizeof(arg2) - 1);
            processRegisterCommand(arg1, arg2);
            break;

        case COMMAND_CONNECT:
            if (nArgs != 3)
            {
                show_help_messages();
                break;
            }
            if (!isClientRegistered)
            {
                cout << "Please register to the server" << endl;
                break;
            }
            strcpy(arg1, strtok(NULL, " "));
            strcpy(arg2, strtok(NULL, " "));
            processConnectCommand(arg1, arg2);
            break;

        case COMMAND_MSG:
            if (!isClientRegistered)
            {
                cout << "MSG Error: Please register to the client first!" << endl;
                break;
            }
            if (nArgs != 2)
            {
                show_help_messages();
                break;
            }
            cout << strtok(NULL, " ");
            processMessageCommand(commandLine + 4);
            break;

        default:
            show_help_messages();
            break;
        }
        memset(arg1, 0, sizeof(arg1));
        memset(arg2, 0, sizeof(arg2));
        free(argLine);
    }
	
    void processNewConnection() {
        int newConnectionSocketDescriptor;
        char remoteIP[32];
        struct sockaddr_in remoteaddr;
        memset(&remoteaddr, 0, sizeof(remoteaddr));
        remoteaddr.sin_family = AF_INET;
        int addrlen = sizeof(remoteaddr);
        if ((newConnectionSocketDescriptor = accept(listeningSocketDescriotor, (struct sockaddr *)&remoteaddr, (socklen_t *)&addrlen)) == -1)
        {
            perror("Error in connection");
            exit(EXIT_FAILURE);
        }
        clientSocketDescriptor = newConnectionSocketDescriptor;
        FD_SET(clientSocketDescriptor, &globalFieldDescriptor);
        if (clientSocketDescriptor > m_nMaxFd)
            m_nMaxFd = clientSocketDescriptor;
        printf("New connection from %s on "
            "socket %d\n",
            inet_ntop(AF_INET, &remoteaddr.sin_addr, remoteIP, 32), newConnectionSocketDescriptor);
        startChat();
    }

	
private:
    void processMessageCommand(char *message) {
        cout << message << endl;
        int len = strlen(message);
        if (sendDataOnSocket(clientSocketDescriptor, message, &len) == -1)
        {
            cout << "Error in sending message, Please retry" << endl;
            return;
        }
    }
    
    int sendDataOnSocket(int sockFd, char *buf, int *length) {
        int totalBytes = 0;
        int remainingBytes = *length;
        int n;
        while (totalBytes < *length)
        {
            n = send(sockFd, buf + totalBytes, remainingBytes, 0);
            if (n == -1)
            {
                break;
            }
            totalBytes += n;
            remainingBytes -= n;
        }
        *length = totalBytes;
        if (n == -1)
            return -1;
        return 0;
    }

    void startChat() {
        cout << "***** Start Chatting *****" << endl;
        for (;;)
        {
            readFieldDescriptor = globalFieldDescriptor;
            if (select(m_nMaxFd + 1, &readFieldDescriptor, NULL, NULL, NULL) == -1)
            {
                perror("Error in connecting");
                exit(EXIT_FAILURE);
            }
            int bytesRead;
            char recvBuff[1024];
            for (int i = 0; i <= m_nMaxFd; i++)
            {
                if (FD_ISSET(i, &readFieldDescriptor))
                {
                    if (i == 0)
                    {
                        string s;
                        getline(cin, s);
                        send(clientSocketDescriptor, s.c_str(), s.length(), 0);
                    }
                    else
                    {
                        if ((bytesRead = recv(i, recvBuff, sizeof(recvBuff), 0)) > 0)
                        {
                            recvBuff[strlen(recvBuff)] = 0;
                            cout << "Remote Message: " << recvBuff << endl;
                        }
                        memset(recvBuff, 0, sizeof(recvBuff));
                    }
                }
            }
        }
    }

    void processConnectCommand(char *ip, char *portNumber) {
        struct sockaddr_in peerSocketAddress;
        char peerIpAddr[32];
        int peerPort = atoi(portNumber);
        int peerSd;
        cout << ip << " " << peerPort << endl;
        strcpy(peerIpAddr, ip);

        memset(&peerSocketAddress, 0, sizeof(peerSocketAddress));
        peerSocketAddress.sin_family = AF_INET;
        peerSocketAddress.sin_port = htons(peerPort);

        if (inet_pton(AF_INET, peerIpAddr, &peerSocketAddress.sin_addr) != 1)
        {
            perror("processConnectCommand inet_pton");
            show_help_messages();
        }
        if ((peerSd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket connection error");
            return;
        }
        if (connect(peerSd, (struct sockaddr *)&peerSocketAddress, sizeof(peerSocketAddress)) < 0)
        {
            perror("Socket connection error");
            return;
        }
        clientSocketDescriptor = peerSd;
        FD_SET(clientSocketDescriptor, &globalFieldDescriptor);
        if (clientSocketDescriptor > m_nMaxFd)
            m_nMaxFd = clientSocketDescriptor;
        startChat();
        close(clientSocketDescriptor);
        cout << "Secure connection established with client" << endl;
    }

    int processRegisterCommand(char *ipAddress, char *portNumber) {
        struct sockaddr_in serverSocketAddress;
        
        int serverPort = atoi(portNumber);
        strcpy(serverIPAddress, ipAddress);

        SSL_library_init();
        SSL_CTX *sslContext = InitializeSSLContext();

        memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));
        serverSocketAddress.sin_family = AF_INET;
        serverSocketAddress.sin_port = htons(serverPort);
        if (inet_pton(AF_INET, serverIPAddress, &serverSocketAddress.sin_addr) != 1)
        {
            perror("processRegisterCommand inet_pton");
            show_help_messages();
        }

        if ((serverSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket");
            return 0;
        }

        if (connect(serverSocketDescriptor, (struct sockaddr *)&serverSocketAddress, sizeof(serverSocketAddress)) < 0)
        {
            perror("connect");
            return 0;
        }
        char buf[1024];
        SSL *sslInstance = SSL_new(sslContext);
        SSL_set_fd(sslInstance, serverSocketDescriptor);
        if (SSL_connect(sslInstance) == 0) {
            ERR_print_errors_fp(stderr);
        }
        else
        {
            char msg[] = "Hello???";
            SSL_write(sslInstance, msg, strlen(msg));
            int bytes = SSL_read(sslInstance, buf, sizeof(buf));
            buf[bytes] = 0;
            SSL_free(sslInstance);
        }
        cout << "Server-Client SSL authentication completed" << endl;
        isClientRegistered = true;
        close(serverSocketDescriptor);
        SSL_CTX_free(sslContext);
        return 1;
    }
	
    int getArgumentsCount(char *line, const char *delim) {
        char *tokens = strtok(line, delim);
        int arguments = 0;
        while (tokens != NULL)
        {
            arguments++;
            tokens = strtok(NULL, delim);
        }
        return arguments;
    }

    CommandID getCommand(char comnd[]) {
        if (strcasecmp(comnd, "REGISTER") == 0)
            return COMMAND_REGISTER;
        else if (strcasecmp(comnd, "CONNECT") == 0)
            return COMMAND_CONNECT;
        else if (strcasecmp(comnd, "MSG") == 0)
            return COMMAND_MSG;
        else
            return COMMAND_NONE;
    }

    SSL_CTX* InitializeSSLContext(void) {
        
        const SSL_METHOD *method = TLS_client_method();
        SSL_CTX *context;

        SSL_load_error_strings();
        OpenSSL_add_all_algorithms(); 
        
        context = SSL_CTX_new(method);
        if (context == NULL)
        {
            ERR_print_errors_fp(stderr);
            abort();
        }
        return context;
    }
};

