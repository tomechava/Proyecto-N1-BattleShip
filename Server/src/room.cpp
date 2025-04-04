#include "../include/room.h"
#include <iostream>
#include <unistd.h>   // Para close() en Linux
#include <string>     // Para string
#include <cstring>    // Para memset
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

using namespace std;

Room::Room(int player1_socket, int player2_socket)
    : player1_socket(player1_socket), player2_socket(player2_socket) {}

void Room::run() {
    cout << "Room creada entre dos jugadores." << endl;

    // Mensajes simples para cada jugador
    string mensajeJugador1 = "Estás en una sala. Espera el turno del otro jugador.\n";
    string mensajeJugador2 = "Estás en una sala. Empiezas tú.\n";

#ifdef _WIN32
    send(player1_socket, mensajeJugador1.c_str(), mensajeJugador1.size(), 0);
    send(player2_socket, mensajeJugador2.c_str(), mensajeJugador2.size(), 0);
#else
    ssize_t sent1 = send(player1_socket, mensajeJugador1.c_str(), mensajeJugador1.size(), 0);
    if (sent1 == -1) {
        perror("Error al enviar mensaje al jugador 1");
    }
    ssize_t sent2 = send(player2_socket, mensajeJugador2.c_str(), mensajeJugador2.size(), 0);
    if (sent2 == -1) {
        perror("Error al enviar mensaje al jugador 2");
    }
#endif

    // Comunicación básica entre los dos jugadores (tipo ping-pong)
    char buffer[1024];         // Buffer para recibir mensajes
    while (true) {
        memset(buffer, 0, sizeof(buffer));         // Limpia el buffer
        ssize_t bytesReceived2 = recv(player2_socket, buffer, sizeof(buffer), 0);    // Recibe mensaje del jugador 2
        if (bytesReceived2 <= 0) break; // Cliente desconectado
#ifdef _WIN32
        send(player1_socket, buffer, bytesReceived2, 0);     // Envía mensaje al jugador 1
#else
        ssize_t sent1_loop = send(player1_socket, buffer, bytesReceived2, 0);
        if (sent1_loop == -1) {
            perror("Error al enviar al jugador 1");
            break;
        }
#endif

        memset(buffer, 0, sizeof(buffer));         // Limpia el buffer
        ssize_t bytesReceived1 = recv(player1_socket, buffer, sizeof(buffer), 0);      // Recibe mensaje del jugador 1
        if (bytesReceived1 <= 0) break; // Cliente desconectado
#ifdef _WIN32
        send(player2_socket, buffer, bytesReceived1, 0);     // Envía mensaje al jugador 2
#else
        ssize_t sent2_loop = send(player2_socket, buffer, bytesReceived1, 0);
        if (sent2_loop == -1) {
            perror("Error al enviar al jugador 2");
            break;
        }
#endif
    }

    cout << "Una sala ha finalizado porque un jugador se desconectó." << endl;

    // Cerrar sockets al terminar
#ifdef _WIN32
    closesocket(player1_socket);
    closesocket(player2_socket);
#else
    close(player1_socket);
    close(player2_socket);
#endif
}