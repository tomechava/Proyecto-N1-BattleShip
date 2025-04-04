#include "../include/room.h"
#include "../include/client_handler.h"
#include "../include/utils.h"
#include <iostream>
#include <unistd.h>   // Para close() en Linux
#include <string>     // Para string
#include <cstring>    // Para memset


using namespace std;

//Constructor de la clase Room
Room::Room(int player1_socket, int player2_socket)
    : player1_socket(player1_socket), player2_socket(player2_socket) {}


// Método que ejecuta la sala de juego
void Room::run() {
    cout << "Room creada entre dos jugadores." << endl;

    // Mensajes simples para cada jugador
    string mensajeJugador1 = "Estás en una sala. Espera el turno del otro jugador.\n";
    string mensajeJugador2 = "Estás en una sala. Empiezas tú.\n";

#ifdef _WIN32   // En Windows, se usa send() directamente
    send(player1_socket, mensajeJugador1.c_str(), mensajeJugador1.size(), 0);
    send(player2_socket, mensajeJugador2.c_str(), mensajeJugador2.size(), 0);
#else   // En Linux, se usa send() con manejo de errores
    // Enviar mensajes a los jugadores
    ssize_t sent1 = send(player1_socket, mensajeJugador1.c_str(), mensajeJugador1.size(), 0);
    if (sent1 == -1) {
        perror("Error al enviar mensaje al jugador 1");
    }
    ssize_t sent2 = send(player2_socket, mensajeJugador2.c_str(), mensajeJugador2.size(), 0);
    if (sent2 == -1) {
        perror("Error al enviar mensaje al jugador 2");
    }
#endif

    // Lanzamos un hilo para manejar los mensajes del jugador 1
    thread t1(handleClientMessages, player1_socket);

    // Lanzamos un hilo para manejar los mensajes del jugador 2
    thread t2(handleClientMessages, player2_socket);

    // Esperamos que ambos terminen
    t1.join();
    t2.join();

    cout << "Una sala ha finalizado porque un jugador se desconectó." << endl;


    // Cerrar sockets al terminar
    CLOSE_SOCKET(player1_socket);
    CLOSE_SOCKET(player2_socket);
}