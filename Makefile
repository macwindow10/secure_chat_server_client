all: secure_chat_application

secure_chat_application: SecureChatApplication.o SecureServer.o SecureClient.o
	g++ -g SecureChatApplication.o SecureServer.o SecureClient.o -o secure_chat_application -lssl -lcrypto

SecureChatApplication.o: SecureChatApplication.cpp
	g++ -g -c SecureChatApplication.cpp

SecureServer.o: SecureServer.cpp
	g++ -g -c SecureServer.cpp 

SecureClient.o: SecureClient.cpp
	g++ -g -c SecureClient.cpp 

clean:
	rm *.o secure_chat_application
