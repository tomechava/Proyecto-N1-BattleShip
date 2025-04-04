#include "../include/server.h"              // Declaración de la clase Server
#include "../include/client_handler.h"      // Función para manejar la conexión de cada cliente
#include "../include/room.h"              // Clase para manejar las salas de juego
#include "../include/protocol.h"          // Protocolo de comunicación entre cliente y servidor
#include "../include/utils.h"
#include <thread>                // Librería estándar para crear y gestionar hilos

using namespace std;             // Evita tener que escribir std::string, std::cout, etc.

// Constructor de la clase Server
Server::Server() : server_fd(0), addrlen(sizeof(address)) {
#ifdef _WIN32
    // Si estamos en Windows, se necesita inicializar la librería Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        exit(EXIT_FAILURE);
    }
#endif
}

// Destructor de la clase Server
Server::~Server() {
    closeServer(); // Cierra el socket del servidor cuando el objeto Server se destruye
}

// Método que inicializa el socket del servidor
bool Server::initialize() {
    // Crea el socket: IPv4, tipo stream (TCP), protocolo por defecto
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        cerr << "Error al crear el socket" << endl;
        return false;
    }

    // Define los parámetros del socket: familia, dirección y puerto
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Acepta conexiones desde cualquier IP
    address.sin_port = htons(PORT);        // Establece el puerto en orden de bytes de red

    // Asocia el socket a la dirección y puerto especificados
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Error al enlazar el socket" << endl;
        return false;
    }

    // Pone el socket en modo escucha (aceptará conexiones)
    if (listen(server_fd, 5) < 0) {
        cerr << "Error al escuchar en el socket" << endl;
        return false;
    }

    cout << "Servidor escuchando en el puerto " << PORT << endl;
    return true;
}

// Método principal que acepta nuevas conexiones y lanza un hilo para cada cliente
void Server::run() {
    while (true) {
        // Espera por una conexión entrante. Cuando un cliente se conecta, se devuelve un nuevo socket
        int client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            cerr << "Error al aceptar la conexión" << endl;
            continue; // Si falla, no detenemos el servidor, solo seguimos esperando
        }

        cout << "Cliente conectado, lanzando hilo..." << endl;

        // Crea un hilo para manejar a este cliente. `handleClient` es una función definida aparte.
        thread clientThread(&Server::handleClient, this, client_socket);

        // El hilo se separa del hilo principal y se ejecuta en segundo plano
        clientThread.detach();  // Evita tener que hacer `join()`, se limpia automáticamente al terminar
    }
}

// Método que cierra el socket del servidor (según si es Linux o Windows)
void Server::closeServer() {
    CLOSE_SOCKET(server_fd);
#ifdef _WIN32
    WSACleanup();           // Libera recursos de Winsock
#endif
}

// Función principal (entry point)
int main() {
    Server server; // Crea instancia de Server

    if (!server.initialize()) { // Inicializa el socket del servidor
        return EXIT_FAILURE;    // Si algo falla, salimos con código de error
    }

    server.run(); // Empieza a aceptar conexiones
    return EXIT_SUCCESS;
}

//Función que maneja la conexión de cada cliente
void Server::handleClient(int client_socket) {    
    // Bloqueamos el mutex para modificar la lista de espera
    clientsMutex.lock();
    waitingClients.push_back(client_socket);    // Agregamos el socket del cliente a la lista de espera

    // Si hay al menos dos clientes, emparejamos
    if (waitingClients.size() >= 2) {
        // Tomamos los dos primeros de la lista
        int client1 = waitingClients[0];
        int client2 = waitingClients[1];

        // Los removemos del vector
        waitingClients.erase(waitingClients.begin());
        waitingClients.erase(waitingClients.begin()); // otra vez en el índice 0

        // Desbloqueamos antes de iniciar la room
        clientsMutex.unlock();

        cout << "Emparejando a dos clientes en una nueva sala..." << endl;

        // Creamos la sala y la ejecutamos en un nuevo hilo
        thread gameThread([client1, client2]() {
            Room room(client1, client2);
            room.run(); // Aquí iría tu lógica de la partida
        });
        gameThread.detach();  // Lo dejamos ejecutarse por sí solo
    } else {
        // Si aún no hay suficiente, desbloqueamos y el cliente esperará
        clientsMutex.unlock();
        cout << "Cliente agregado a la lista de espera. Esperando oponente..." << endl;
    }
}