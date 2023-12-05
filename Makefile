all: secure_chat_application

secure_chat_application: SecureChatApplication.o SecureServer.o client.o
	g++ -g SecureChatApplication.o SecureServer.o client.o -o secure_chat_application -lssl -lcrypto

SecureChatApplication.o: SecureChatApplication.cpp
	g++ -g -c SecureChatApplication.cpp

SecureServer.o: SecureServer.cpp
	g++ -g -c SecureServer.cpp 

client.o: client.cpp
	g++ -g -c client.cpp 

clean:
	rm *.o secure_chat_application
