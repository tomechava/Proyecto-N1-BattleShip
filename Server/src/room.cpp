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
        case MessageType::READY:
            handleReady(playerSocket, msg);
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


// Manejo de la señal de "listo"
void Room::handleReady(int playerSocket, const ProtocolMessage& msg) {
    if (playerSocket == player1_socket) player1_boats = convertBoats(msg.data);
    if (playerSocket == player2_socket) player2_boats = convertBoats(msg.data); 
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
    auto [hit, sunk] = applyFire(playerSocket, cell);

    MessageType result;
    if (sunk) {
        result = MessageType::SUNK;
    } else {
        result = hit ? MessageType::HIT : MessageType::MISS;
    }

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

// Manejo de xla señal de "chat"
void Room::handleChat(int senderSocket, const ProtocolMessage& msg) {
    if (!msg.data.empty()) {
        string chatMsg = "Jugador dice: " + msg.data[0] + "\n";
        int receiverSocket = (senderSocket == player1_socket) ? player2_socket : player1_socket;
        send(receiverSocket, chatMsg.c_str(), chatMsg.size(), 0);
    }
}

// Metodo para agregar las casillas jugadas a un arreglo (individual por cada player)
void Room::addSelectedCell(int playerSocket, const std::string&cell ){
    if (playerSocket == player1_socket){
        player1_selected_cells.push_back(cell);
    }else if (playerSocket == player2_socket){
        player2_selected_cells.push_back(cell);
    }
    
}

// Aplicar el disparo a la celda
std::pair<bool, bool> Room::applyFire(int attackerSocket, const std::string& cell) {
    bool hit = false;
    bool sunk = false;

    auto& opponent_boats = (attackerSocket == player1_socket) ? player2_boats : player1_boats;
    auto& opponent_selected_cells = (attackerSocket == player1_socket) ? player2_selected_cells : player1_selected_cells;

    for (const auto& boat : opponent_boats) {
        if (std::find(boat.begin(), boat.end(), cell) != boat.end()) {
            hit = true;
            break;
        }
    }

    if (hit) {
        opponent_selected_cells.push_back(cell);

        // Revisar si algún barco fue hundido completamente
        for (const auto& boat : opponent_boats) {
            bool all_hit = true;
            for (const auto& part : boat) {
                if (std::find(opponent_selected_cells.begin(), opponent_selected_cells.end(), part) == opponent_selected_cells.end()) {
                    all_hit = false;
                    break;
                }
            }
            if (all_hit && std::find(boat.begin(), boat.end(), cell) != boat.end()) {
                sunk = true;
                break;
            }
        }
    }

    logWithTimestamp("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)") + (sunk ? " (SUNK)" : ""));
    return {hit, sunk};
}

// Verificar si el jugador ha ganado
bool Room::checkVictory(int attackerSocket) {
    const auto& board = (attackerSocket == player1_socket) ? player2_board : player1_board;
    const auto& selected_cells = (attackerSocket == player1_socket) ? player2_selected_cells : player1_selected_cells;
    for (const auto& boat : board) {
        for (const auto& coord : boat) {
            if (find(selected_cells.begin(), selected_cells.end(), coord) == selected_cells.end()) {
                return false; // Coordenada no encontrada
            }
        }
    }
    return true;
}

// Método que maneja el bucle del juego
void Room::gameLoop() {
    logWithTimestamp("Loop del juego iniciado. Turnos serán manejados por los mensajes FIRE.");
}

//Transformar msg de botes a arreglo bidimensional
vector<vector<string>> convertBoats(const vector<string>& data) {
    vector<vector<string>> resultado;

    for (const string& grupo : data) {
        vector<string> subgrupo;
        stringstream ss(grupo);
        string token;

        while (getline(ss, token, ',')) {
            subgrupo.push_back(token);
        }

        resultado.push_back(subgrupo);
    }

    return resultado;
}