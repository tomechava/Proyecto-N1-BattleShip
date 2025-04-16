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
#include <algorithm>  // Para std::find
#include <sstream>   // Para std::stringstream
#include <set>
#include <vector>
#include <tuple>

using namespace std;

//Constructor de la clase Room
Room::Room(int player1_socket, int player2_socket)
    : player1_socket(player1_socket), player2_socket(player2_socket) {}


// Método que ejecuta la sala de juego
void Room::run() {
    logWithTimestamp("Room creada entre dos jugadores.");
    LOG_ROOM("Room creada entre dos jugadores.");

    // Inicializa los estados
    player1_ready = false;
    player2_ready = false;

    // Mensajes simples para cada jugador
    string mensajeJugador = "Bienvenido. Esperando a que coloques tus barcos.\n";

    //Enviar REGISTER y mensajeJugador a cada jugador
    ProtocolMessage msg = { MessageType::REGISTER, {} };

    msg.data.push_back(mensajeJugador);
    string room_joined_msg = createMessage(MessageType::REGISTER, msg.data);

    // Enviar mensajes a los jugadores
    ssize_t sent1 = send(player1_socket, room_joined_msg.c_str(), room_joined_msg.size(), 0);
    if (sent1 == -1) {
        perror("Error al enviar mensaje REGISTER al jugador 1");
        LOG_ROOM("Error al enviar mensaje REGISTER al jugador 1");
    } else {
        logWithTimestamp("Mensaje REGISTER enviado al jugador 1.");
        LOG_ROOM(room_joined_msg + " enviado al jugador 1.");
    }
    ssize_t sent2 = send(player2_socket, room_joined_msg.c_str(), room_joined_msg.size(), 0);
    if (sent2 == -1) {
        perror("Error al enviar mensaje REGISTER al jugador 2");
        LOG_ROOM("Error al enviar mensaje REGISTER al jugador 2");
    } else {
        logWithTimestamp("Mensaje REGISTER enviado al jugador 2.");
        LOG_ROOM(room_joined_msg + " enviado al jugador 2.");
    }

    // Iniciar hilos para escuchar mensajes de los jugadores
    std::thread t1([this]() {
        handleClientMessages(player1_socket, this); // <- le pasamos la room
    });
    std::thread t2([this]() {
        handleClientMessages(player2_socket, this);
    });

    // Esperar que ambos jugadores estén listos
    waitForPlayersReady();

    // Enviar mensaje de inicio a ambos jugadores
    ProtocolMessage start_msg = { MessageType::READY, {} };
    string start_message = createMessage(MessageType::READY, start_msg.data);
    ssize_t sent_start1 = send(player1_socket, start_message.c_str(), start_message.size(), 0);
    if (sent_start1 == -1) {
        perror("Error al enviar mensaje de inicio al jugador 1");
        LOG_ROOM("Error al enviar mensaje de inicio al jugador 1");
    } else {
        logWithTimestamp("Mensaje de inicio enviado al jugador 1.");
        LOG_ROOM(start_message + " enviado al jugador 1.");
    }
    ssize_t sent_start2 = send(player2_socket, start_message.c_str(), start_message.size(), 0);
    if (sent_start2 == -1) {
        perror("Error al enviar mensaje de inicio al jugador 2");
        LOG_ROOM("Error al enviar mensaje de inicio al jugador 2");
    } else {
        logWithTimestamp("Mensaje de inicio enviado al jugador 2.");
        LOG_ROOM(start_message + " enviado al jugador 2.");
    }

    logWithTimestamp("Ambos jugadores listos. ¡Comienza el juego!");
    LOG_ROOM("Ambos jugadores listos. ¡Comienza el juego!");

    current_turn_socket = player2_socket; // por ejemplo, el player 2 comienza
    other_player_socket = player1_socket;

    // Enviar mensaje TURN al jugador que comienza
    ProtocolMessage turn_msg = { MessageType::TURN, {"Es tu turno."} };
    string msg_str = createMessage(turn_msg.type, turn_msg.data);
    ssize_t sent_turn = send(current_turn_socket, msg_str.c_str(), msg_str.size(), 0);
    if (sent_turn == -1) {
        perror("Error al enviar mensaje de turno al jugador 1");
        LOG_ROOM("Error al enviar mensaje de turno al jugador 1");
    } else {
        logWithTimestamp("Mensaje de turno enviado al jugador 1.");
        LOG_ROOM(msg_str + " enviado al jugador 1.");
    }

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
    string msg_str = "";
    std::lock_guard<std::mutex> lock(game_mutex);

    switch (msg.type) {
        case MessageType::READY:
            msg_str = createMessage(MessageType::READY, msg.data);
            LOG_ROOM(msg_str + " recibido del jugador.");
            handleReady(playerSocket, msg);
            break;
        case MessageType::FIRE:
            msg_str = createMessage(MessageType::FIRE, msg.data);
            LOG_ROOM(msg_str + " recibido del jugador.");
            handleFire(playerSocket, msg);
            break;
        default:
            break;
    }
}


// Manejo de la señal de "listo"
void Room::handleReady(int playerSocket, const ProtocolMessage& msg) {
    if (playerSocket == player1_socket) player1_boats = convertBoats(joinVector(msg.data, ","));
    if (playerSocket == player2_socket) player2_boats = convertBoats(joinVector(msg.data, ",")); 
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


    vector<set<string>> applyfire_boats = (playerSocket == player1_socket) ? player2_boats : player1_boats;
    auto [hit, sunk] = applyFire(cell, applyfire_boats);  // Destructuración válida

    LOG_ROOM("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)") + (sunk ? " (SUNK)" : ""));

    MessageType result;
    if (sunk) {
        result = MessageType::SUNK;
    } else {
        result = hit ? MessageType::HIT : MessageType::MISS;
    }

    cout << "Resultado del disparo: " << (hit ? "HIT" : "MISS") << endl;

    string response = createMessage(result, { cell });

    // Enviar el resultado a ambos jugadores
    if (send(playerSocket, response.c_str(), response.size(), 0) == -1) {
        perror("Error al enviar al jugador que disparó");
    }
    if (send(other_player_socket, response.c_str(), response.size(), 0) == -1) {
        perror("Error al enviar al jugador objetivo");
    }

    logWithTimestamp("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)") + (sunk ? " (SUNK)" : ""));

    // Verifica si el oponente ha perdido
    if (checkVictory(other_player_socket)) {
        handleVictory(playerSocket);  // playerSocket gana
        return;
    }

    // Cambiar turno
    swap(current_turn_socket, other_player_socket);

    // Enviar mensaje TURN al nuevo jugador
    ProtocolMessage turn_msg = { MessageType::TURN, {"Es tu turno."} };
    string msg_str = createMessage(turn_msg.type, turn_msg.data);
    if (send(current_turn_socket, msg_str.c_str(), msg_str.size(), 0) == -1) {
        perror("Error al enviar mensaje de turno");
    }

    logWithTimestamp("Turno cambiado.");
    LOG_ROOM("Turno cambiado. Es el turno del jugador " + to_string(current_turn_socket));
}


// Manejo de la señal de "victoria"
void Room::handleVictory(int winnerSocket) {
    string winMsg = createMessage(MessageType::WIN, {});
    string loseMsg = createMessage(MessageType::LOSE, {});

    send(winnerSocket, winMsg.c_str(), winMsg.size(), 0);
    int loserSocket = (winnerSocket == player1_socket) ? player2_socket : player1_socket;
    send(loserSocket, loseMsg.c_str(), loseMsg.size(), 0);

    logWithTimestamp("Juego terminado. ¡Hay un ganador!");
    LOG_ROOM("Juego terminado. ¡Hay un ganador!");
}




tuple<bool, bool> applyFire(const string& cell, vector<set<string>>& boats) {
    for (auto it = boats.begin(); it != boats.end(); ++it) {
        if (it->count(cell)) {
            it->erase(cell);  // quitamos la celda golpeada
            bool sunk = it->empty();  // si quedó vacío, se hundió
            if (sunk) {
                boats.erase(it);  // eliminamos el barco hundido de la lista
            }
            return {true, sunk};
        }
    }
    return {false, false};  // No fue hit
}


// Verificar si el jugador ha ganado
bool Room::checkVictory(int attackerSocket) {
    const auto& boats = (attackerSocket == player1_socket) ? player2_boats : player1_boats;
    for (const auto& boat : boats) {
        if (!boat.empty()) {
            return false;  // Todavía hay partes de barcos sin hundir
        }
    }
    return true;  // Todos los barcos están hundidos
}

void Room::gameLoop() {
    logWithTimestamp("🎮 Iniciando bucle del juego...");
    LOG_ROOM("🎮 Iniciando bucle del juego...");

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::lock_guard<std::mutex> lock(game_mutex);

            // Si ya hay un ganador, salimos del loop
            if (checkVictory(player1_socket)){
                handleVictory(player1_socket);
                break;
            } else if (checkVictory(player2_socket)) {
                handleVictory(player2_socket);
                break;
            }
            // Si no hay ganador, seguimos esperando

            // En este punto no hacemos nada más, ya que los turnos
            // y disparos se gestionan desde handleFire.
            // Solo estamos esperando a que el juego termine.
        }
    

    logWithTimestamp("🏁 Juego finalizado en la sala.");
    LOG_ROOM("🏁 Juego finalizado en la sala.");
}

vector<set<string>> convertBoats(const std::string& data) {
    vector<set<string>> boats;
    stringstream ss(data);
    string boat_str;

    while (getline(ss, boat_str, ',')) {
        set<string> boat;
        stringstream boat_ss(boat_str);
        string cell;

        while (getline(boat_ss, cell, '-')) {
            boat.insert(cell);
        }

        if (!boat.empty()) {
            boats.push_back(boat);
        }
    }

    logWithTimestamp("Barcos convertidos: " + vectorOfSetsToString(boats));
    LOG_ROOM("Barcos convertidos: " + vectorOfSetsToString(boats));

    return boats;
}