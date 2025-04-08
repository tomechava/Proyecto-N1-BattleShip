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
    logWithTimestamp("Room creada entre dos jugadores.");

    // Inicializa los estados
    player1_ready = false;
    player2_ready = false;

    // Mensajes simples para cada jugador
    string mensajeJugador1 = "Bienvenido. Esperando a que coloques tus barcos.\n";
    string mensajeJugador2 = "Bienvenido. Esperando a que coloques tus barcos.\n";

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

    // Iniciar hilos para escuchar mensajes de los jugadores
    std::thread t1([this]() {
        handleClientMessages(player1_socket, this); // <- le pasamos la room
    });
    std::thread t2([this]() {
        handleClientMessages(player2_socket, this);
    });

    // Esperar que ambos jugadores estén listos
    waitForPlayersReady();

    logWithTimestamp("Ambos jugadores listos. ¡Comienza el juego!");

    current_turn_socket = player2_socket; // por ejemplo, el player 2 comienza
    other_player_socket = player1_socket;

    gameLoop();

    // Esperamos que ambos terminen
    t1.join();
    t2.join();

    cout << "Una sala ha finalizado." << endl;


    // Cerrar sockets al terminar
    CLOSE_SOCKET(player1_socket);
    CLOSE_SOCKET(player2_socket);
}


void Room::waitForPlayersReady() {
    while (!player1_ready || !player2_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


void Room::onPlayerMessage(int playerSocket, const ProtocolMessage& msg) {
    std::lock_guard<std::mutex> lock(game_mutex);

    switch (msg.type) {
        case MessageType::READY:
            if (playerSocket == player1_socket) player1_ready = true;
            if (playerSocket == player2_socket) player2_ready = true;
            break;

        case MessageType::FIRE:
            if (playerSocket == current_turn_socket) {
                // TODO: Validar disparo y cambiar turno
                string cell = msg.data[0]; // por ejemplo, "B4"
                logWithTimestamp("Jugador disparó a " + cell);

                // Simulación simple:
                string response = createMessage(MessageType::MISS, { cell });
                send(current_turn_socket, response.c_str(), response.size(), 0);
                send(other_player_socket, response.c_str(), response.size(), 0);

                std::swap(current_turn_socket, other_player_socket);
            } else {
                string warning = "No es tu turno.\n";
                send(playerSocket, warning.c_str(), warning.size(), 0);
            }
            break;

        default:
            // Ignorar o loggear otros tipos
            break;
    }
}