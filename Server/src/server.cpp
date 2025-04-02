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

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error al inicializar Winsock" << std::endl;
        return -1;
    }
#endif

    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Crear el socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Error al crear el socket" << std::endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Error al enlazar el socket" << std::endl;
        return -1;
    }

    // Poner el socket en modo escucha
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Error al escuchar en el socket" << std::endl;
        return -1;
    }

    std::cout << "Servidor escuchando en el puerto " << PORT << std::endl;

    // Aceptar conexiones
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0) {
        std::cerr << "Error al aceptar la conexión" << std::endl;
        return -1;
    }
    std::cout << "Cliente conectado" << std::endl;

    std::string message = "Bienvenido al servidor!\n";
    send(new_socket, message.c_str(), message.size(), 0);

    // Cerrar el socket
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif

    return 0;
}
