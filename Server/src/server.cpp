#include "../include/server.h"              // Declaración de la clase Server
#include "../include/client_handler.h"      // Función para manejar la conexión de cada cliente
#include "../include/room.h"                // Clase para manejar las salas de juego
#include "../include/protocol.h"            // Protocolo de comunicación entre cliente y servidor
#include "../include/utils.h"               // Funciones utilitarias como logToFile
#include <thread>

using namespace std;

// Constructor
Server::Server() : server_fd(0), addrlen(sizeof(address)) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        logToFile("server.log", "Error al inicializar Winsock");
        exit(EXIT_FAILURE);
    }
#endif
}

// Destructor
Server::~Server() {
    closeServer();
}

// Inicializar socket del servidor
bool Server::initialize() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        cerr << "Error al crear el socket" << endl;
        logToFile("server.log", "Error al crear el socket");
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Error al enlazar el socket" << endl;
        logToFile("server.log", "Error al enlazar el socket");
        return false;
    }

    if (listen(server_fd, 5) < 0) {
        cerr << "Error al escuchar en el socket" << endl;
        logToFile("server.log", "Error al escuchar en el socket");
        return false;
    }

    string msg = "Servidor escuchando en el puerto " + to_string(PORT);
    cout << msg << endl;
    logToFile("server.log", msg);

    return true;
}

// Acepta conexiones y lanza hilo por cliente
void Server::run() {
    while (true) {
        int client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            cerr << "Error al aceptar la conexión" << endl;
            logToFile("server.log", "Error al aceptar la conexión");
            continue;
        }

        string msg = "Cliente conectado (socket " + to_string(client_socket) + ")";
        cout << msg << endl;
        logToFile("server.log", msg);

        thread clientThread(&Server::handleClient, this, client_socket);
        clientThread.detach();
    }
}

// Cierra el socket del servidor
void Server::closeServer() {
    CLOSE_SOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    logToFile("server.log", "Servidor cerrado");
}

// Punto de entrada
int main() {
    Server server;

    if (!server.initialize()) {
        return EXIT_FAILURE;
    }

    server.run();
    return EXIT_SUCCESS;
}

// Manejo de un cliente individual
void Server::handleClient(int client_socket) {
    clientsMutex.lock();
    waitingClients.push_back(client_socket);

    string log_msg = "Cliente agregado a la lista de espera: socket " + to_string(client_socket);
    cout << log_msg << endl;
    logToFile("server.log", log_msg);

    if (waitingClients.size() >= 2) {
        int client1 = waitingClients[0];
        int client2 = waitingClients[1];

        waitingClients.erase(waitingClients.begin());
        waitingClients.erase(waitingClients.begin());

        clientsMutex.unlock();

        logToFile("server.log", "Emparejando clientes " + to_string(client1) + " y " + to_string(client2));

        thread gameThread([client1, client2]() {
            Room room(client1, client2);
            logToFile("server.log", "Nueva sala creada para sockets " + to_string(client1) + " y " + to_string(client2));
            room.run();
        });
        gameThread.detach();
    } else {
        clientsMutex.unlock();
        logToFile("server.log", "Esperando más jugadores para emparejar...");
    }
}
