#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <string>
#include <vector>        // Para manejar listas dinámicas (vector)
#include <mutex>        // Para manejar la sincronización entre hilos

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
    void handleClient(int client_socket);   // Maneja la conexión de cada cliente
    void closeServer(); 

private:
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen;

    std::vector<int> waitingClients; // Almacena los sockets de clientes en espera
    std::mutex clientsMutex;         // Protege el acceso a waitingClients
};

#endif // SERVER_H
