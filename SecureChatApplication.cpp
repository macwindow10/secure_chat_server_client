
#include "SecureClient.cpp"
#include "SecureServer.cpp"

using namespace std;

void show_help_messages(int argc, char *argv[])
{
    printf("Help: %s application_type port \n", argv[0]);
    printf("application_type: 'server' for server and 'client' for client \n");
    printf("port: listening port server/client \n");
}

int main(int argc, char **argv)
{
    SSL_library_init();
    
    SSL_load_error_strings();

    OpenSSL_add_all_algorithms();
    
    // check number of parameters
    if (argc != 3)
    {
        show_help_messages(argc, argv);
        return -1;
    }

    int portNumber = atoi(argv[2]);

    if (strcmp("server", argv[1]) == 0)
    {
        // run application as server application
        SecureServer secure_server(portNumber);
    }
    else if (strcmp("client", argv[1]) == 0)
    {
        // run application as client application
        SecureClient secure_client(portNumber);
    }
    else
    {
        show_help_messages(argc, argv);
        return -1;
    }

    return 0;
}
