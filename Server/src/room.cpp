#include "../include/room.h"
#include "../include/client_handler.h"
#include "../include/utils.h"
#include <iostream>
#include <unistd.h>   // Para close() en Linux
#include <string>     // Para string
#include <cstring>    // Para memset
#include <thread>
#include <chrono>
#include <map>
#include <mutex>

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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));        //Esperamos hasta que ambos jugadores estén listos
    }
}


void Room::onPlayerMessage(int playerSocket, const ProtocolMessage& msg) {
    std::lock_guard<std::mutex> lock(game_mutex);

    switch (msg.type) {
        case MessageType::PLACE:
            handlePlace(playerSocket, msg);
            break;
        case MessageType::READY:
            handleReady(playerSocket);
            break;
        case MessageType::FIRE:
            handleFire(playerSocket, msg);
            break;
        case MessageType::CHAT:
            handleChat(playerSocket, msg);
            break;
        default:
            break;
    }
}

// -------------------------- FUNCIONES AUXILIARES -----------------------------
//Colocacion de barcos
void Room::handlePlace(int playerSocket, const ProtocolMessage& msg) {
    if (msg.data.empty()) {
        string error = "Error: no se recibieron coordenadas para PLACE.\n";
        send(playerSocket, error.c_str(), error.size(), 0);
        return;
    }

    std::map<std::string, bool>& board = (playerSocket == player1_socket) ? player1_board : player2_board;

    // Evitar que reemplacen una colocación ya hecha
    if (!board.empty()) {
        string error = "Ya colocaste tus barcos.\n";
        send(playerSocket, error.c_str(), error.size(), 0);
        return;
    }

    for (const string& cell : msg.data) {
        // Validación básica: tamaño 2 o 3 (ej. A1, B10)
        if (cell.size() < 2 || cell.size() > 3) {
            string error = "Coordenada inválida: " + cell + "\n";
            send(playerSocket, error.c_str(), error.size(), 0);
            return;
        }
        board[cell] = true;
    }

    string confirm = "Barcos colocados con éxito.\n";
    send(playerSocket, confirm.c_str(), confirm.size(), 0);

    logWithTimestamp("Jugador colocó barcos: " + std::to_string(board.size()) + " celdas.");
}

// Manejo de la señal de "listo"
void Room::handleReady(int playerSocket) {
    if (playerSocket == player1_socket) player1_ready = true;
    if (playerSocket == player2_socket) player2_ready = true;
    logWithTimestamp("Jugador listo.");
}

// Manejo de la señal de "disparar"
void Room::handleFire(int playerSocket, const ProtocolMessage& msg) {
    if (playerSocket != current_turn_socket) {
        string warning = "No es tu turno.\n";
        send(playerSocket, warning.c_str(), warning.size(), 0);
        return;
    }

    if (msg.data.empty()) {
        string error = "FIRE sin coordenada.\n";
        send(playerSocket, error.c_str(), error.size(), 0);
        return;
    }

    string cell = msg.data[0];
    bool hit = applyFire(playerSocket, cell);

    MessageType result = hit ? MessageType::HIT : MessageType::MISS;
    string response = createMessage(result, { cell });

    send(current_turn_socket, response.c_str(), response.size(), 0);
    send(other_player_socket, response.c_str(), response.size(), 0);

    if (checkVictory(playerSocket)) {
        handleVictory(playerSocket);
    } else {
        swap(current_turn_socket, other_player_socket);
    }
}

// Manejo de la señal de "victoria"
void Room::handleVictory(int winnerSocket) {
    string winMsg = createMessage(MessageType::WIN, {});
    string loseMsg = createMessage(MessageType::LOSE, {});

    send(winnerSocket, winMsg.c_str(), winMsg.size(), 0);
    int loserSocket = (winnerSocket == player1_socket) ? player2_socket : player1_socket;
    send(loserSocket, loseMsg.c_str(), loseMsg.size(), 0);

    logWithTimestamp("Juego terminado. ¡Hay un ganador!");
}

// Manejo de la señal de "chat"
void Room::handleChat(int senderSocket, const ProtocolMessage& msg) {
    if (!msg.data.empty()) {
        string chatMsg = "Jugador dice: " + msg.data[0] + "\n";
        int receiverSocket = (senderSocket == player1_socket) ? player2_socket : player1_socket;
        send(receiverSocket, chatMsg.c_str(), chatMsg.size(), 0);
    }
}

// Aplicar el disparo a la celda
bool Room::applyFire(int attackerSocket, const string& cell) {
    bool hit = false;

    if (attackerSocket == player1_socket) {
        hit = player2_board[cell];
        if (hit) player2_board[cell] = false;
    } else {
        hit = player1_board[cell];
        if (hit) player1_board[cell] = false;
    }

    logWithTimestamp("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)"));
    return hit;
}

// Verificar si el jugador ha ganado
bool Room::checkVictory(int attackerSocket) {
    const auto& board = (attackerSocket == player1_socket) ? player2_board : player1_board;
    for (const auto& cell : board) {
        if (cell.second) return false;
    }
    return true;
}

// Método que maneja el bucle del juego
void Room::gameLoop() {
    logWithTimestamp("Loop del juego iniciado. Turnos serán manejados por los mensajes FIRE.");
}