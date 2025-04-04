#include "room.h"
#include <iostream>
#include <unistd.h> // close()

Room::Room(int p1, int p2)      // Constructor de la clase Room
    : player1_socket(p1), player2_socket(p2) {}     // Inicializa los sockets de los jugadores

// Método para iniciar el juego
// Este método crea un hilo que ejecuta la lógica del juego
void Room::startGame() {
    game_thread = std::thread(&Room::handleGame, this);     // Crea un hilo para manejar el juego
    game_thread.detach(); // Permitir que el hilo corra en segundo plano
}

void Room::handleGame() {
    // Aquí irá la lógica de juego: turnos, comunicación, etc.
    std::cout << "Nueva partida entre dos jugadores." << std::endl;

    // Ejemplo simple:
    const char* msg = "¡Partida iniciada!\n";
    send(player1_socket, msg, strlen(msg), 0);      // Enviar mensaje al jugador 1
    send(player2_socket, msg, strlen(msg), 0);      // Enviar mensaje al jugador 2

    // Cerrar sockets cuando termina la partida
    close(player1_socket);
    close(player2_socket);
}
