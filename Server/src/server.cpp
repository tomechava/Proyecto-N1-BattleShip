#include "server.h"              // Declaración de la clase Server
#include "client_handler.h"      // Función para manejar la conexión de cada cliente

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
        thread clientThread(handleClient, client_socket);

        // El hilo se separa del hilo principal y se ejecuta en segundo plano
        clientThread.detach();  // Evita tener que hacer `join()`, se limpia automáticamente al terminar
    }
}

// Método que cierra el socket del servidor (según si es Linux o Windows)
void Server::closeServer() {
#ifdef _WIN32
    closesocket(server_fd); // Cierra socket en Windows
    WSACleanup();           // Libera recursos de Winsock
#else
    close(server_fd);       // Cierra socket en Linux/Mac
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
