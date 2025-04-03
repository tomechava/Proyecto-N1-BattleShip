#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")  // Necesario para linkear Winsock en Visual Studio
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 8080

class Server {
public:
    Server();
    ~Server();
    
    bool initialize();
    void run();
    
private:
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen;

    void closeServer();
    void handleClient(int client_socket);
};

#endif // SERVER_H
