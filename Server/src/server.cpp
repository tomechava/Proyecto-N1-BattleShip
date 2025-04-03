#include "server.h"

using namespace std; // Evita repetir std::

Server::Server() : server_fd(0), addrlen(sizeof(address)) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        exit(EXIT_FAILURE);
    }
#endif
}

Server::~Server() {
    closeServer();
}

bool Server::initialize() {
    // Crear el socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        cerr << "Error al crear el socket" << endl;
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Enlazar el socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Error al enlazar el socket" << endl;
        return false;
    }

    // Poner el socket en modo escucha
    if (listen(server_fd, 5) < 0) {
        cerr << "Error al escuchar en el socket" << endl;
        return false;
    }

    cout << "Servidor escuchando en el puerto " << PORT << endl;
    return true;
}

void Server::run() {
    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            cerr << "Error al aceptar la conexión" << endl;
            continue;
        }
        cout << "Cliente conectado" << endl;
        handleClient(client_socket);
    }
}

void Server::handleClient(int client_socket) {
    string message = "Bienvenido al servidor!\n";
    send(client_socket, message.c_str(), message.size(), 0);
    
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

void Server::closeServer() {
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
}

int main() {
    Server server;

    if (!server.initialize()) {
        return EXIT_FAILURE;
    }

    server.run();
    return EXIT_SUCCESS;
}
