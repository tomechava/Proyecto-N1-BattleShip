#include "../include/room.h"
#include "../include/client_handler.h"
#include "../include/utils.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <map>
#include <mutex>
#include <algorithm>

using namespace std;

Room::Room(int player1_socket, int player2_socket)
    : player1_socket(player1_socket), player2_socket(player2_socket) {}


void Room::run() {
    logWithTimestamp("Room creada entre dos jugadores.");
    logToFile("Sala creada para jugadores " + to_string(player1_socket) + " y " + to_string(player2_socket));

    player1_ready = false;
    player2_ready = false;

    string mensajeJugador = "Bienvenido. Esperando a que coloques tus barcos.\n";

    ProtocolMessage msg = { MessageType::REGISTER, {} };
    msg.data.push_back(mensajeJugador);
    string room_joined_msg = createMessage(MessageType::REGISTER, msg.data);

    ssize_t sent1 = send(player1_socket, room_joined_msg.c_str(), room_joined_msg.size(), 0);
    logToFile("Enviado a jugador 1 (" + to_string(player1_socket) + "): " + room_joined_msg);

    if (sent1 == -1) perror("Error al enviar mensaje al jugador 1");

    ssize_t sent2 = send(player2_socket, room_joined_msg.c_str(), room_joined_msg.size(), 0);
    logToFile("Enviado a jugador 2 (" + to_string(player2_socket) + "): " + room_joined_msg);

    if (sent2 == -1) perror("Error al enviar mensaje al jugador 2");

    std::thread t1([this]() {
        handleClientMessages(player1_socket, this);
    });
    std::thread t2([this]() {
        handleClientMessages(player2_socket, this);
    });

    waitForPlayersReady();

    ProtocolMessage start_msg = { MessageType::READY, {} };
    string start_message = createMessage(MessageType::READY, start_msg.data);

    ssize_t sent_start1 = send(player1_socket, start_message.c_str(), start_message.size(), 0);
    logToFile("Enviado a jugador 1 (inicio del juego): " + start_message);

    if (sent_start1 == -1) perror("Error al enviar mensaje de inicio al jugador 1");

    ssize_t sent_start2 = send(player2_socket, start_message.c_str(), start_message.size(), 0);
    logToFile("Enviado a jugador 2 (inicio del juego): " + start_message);

    if (sent_start2 == -1) perror("Error al enviar mensaje de inicio al jugador 2");

    logWithTimestamp("Ambos jugadores listos. ¡Comienza el juego!");
    logToFile("Ambos jugadores están listos. Comienza el juego.");

    current_turn_socket = player2_socket;
    other_player_socket = player1_socket;

    ProtocolMessage turn_msg = { MessageType::TURN, {"Es tu turno."} };
    string msg_str = createMessage(turn_msg.type, turn_msg.data);

    ssize_t sent_turn = send(current_turn_socket, msg_str.c_str(), msg_str.size(), 0);
    logToFile("Enviado turno a jugador que comienza (" + to_string(current_turn_socket) + "): " + msg_str);

    if (sent_turn == -1) perror("Error al enviar mensaje de turno al jugador 1");

    gameLoop();

    t1.join();
    t2.join();

    logToFile("Sala finalizada para sockets " + to_string(player1_socket) + " y " + to_string(player2_socket));
    cout << "Una sala ha finalizado." << endl;

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

    logToFile("Recibido de jugador " + to_string(playerSocket) + ": " + to_string(msg.type) + " con data -> " + (msg.data.empty() ? "[]" : msg.data[0]));

    switch (msg.type) {
        case MessageType::READY:
            handleReady(playerSocket, msg);
            break;
        case MessageType::FIRE:
            handleFire(playerSocket, msg);
            break;
        default:
            logToFile("Mensaje no reconocido recibido de jugador " + to_string(playerSocket));
            break;
    }
}

void Room::handleReady(int playerSocket, const ProtocolMessage& msg) {
    if (playerSocket == player1_socket) player1_boats = convertBoats(msg.data);
    if (playerSocket == player2_socket) player2_boats = convertBoats(msg.data);
    if (playerSocket == player1_socket) player1_ready = true;
    if (playerSocket == player2_socket) player2_ready = true;

    logWithTimestamp("Jugador listo.");
    logToFile("Jugador " + to_string(playerSocket) + " ha colocado sus barcos.");
}

void Room::handleFire(int playerSocket, const ProtocolMessage& msg) {
    if (playerSocket != current_turn_socket) {
        string warning = "No es tu turno.\n";
        send(playerSocket, warning.c_str(), warning.size(), 0);
        logToFile("Advertencia enviada a jugador " + to_string(playerSocket) + ": No es su turno.");
        return;
    }

    if (msg.data.empty()) {
        string error = "FIRE sin coordenada.\n";
        send(playerSocket, error.c_str(), error.size(), 0);
        logToFile("Error: FIRE sin coordenada de jugador " + to_string(playerSocket));
        return;
    }

    string cell = msg.data[0];
    addSelectedCell(playerSocket, cell);
    auto [hit, sunk] = applyFire(playerSocket, cell);

    MessageType result = sunk ? MessageType::SUNK : (hit ? MessageType::HIT : MessageType::MISS);
    string response = createMessage(result, { cell });

    send(playerSocket, response.c_str(), response.size(), 0);
    send(other_player_socket, response.c_str(), response.size(), 0);

    logWithTimestamp("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)") + (sunk ? " (SUNK)" : ""));
    logToFile("Resultado enviado: " + response);

    if (checkVictory(other_player_socket)) {
        handleVictory(playerSocket);
        return;
    }

    swap(current_turn_socket, other_player_socket);

    ProtocolMessage turn_msg = { MessageType::TURN, {"Es tu turno."} };
    string msg_str = createMessage(turn_msg.type, turn_msg.data);
    send(current_turn_socket, msg_str.c_str(), msg_str.size(), 0);

    logWithTimestamp("Turno cambiado.");
    logToFile("Turno asignado a jugador " + to_string(current_turn_socket));
}

void Room::handleVictory(int winnerSocket) {
    string winMsg = createMessage(MessageType::WIN, {});
    string loseMsg = createMessage(MessageType::LOSE, {});
    int loserSocket = (winnerSocket == player1_socket) ? player2_socket : player1_socket;

    send(winnerSocket, winMsg.c_str(), winMsg.size(), 0);
    send(loserSocket, loseMsg.c_str(), loseMsg.size(), 0);

    logWithTimestamp("Juego terminado. ¡Hay un ganador!");
    logToFile("Jugador " + to_string(winnerSocket) + " ganó. Perdedor: " + to_string(loserSocket));
}

void Room::addSelectedCell(int playerSocket, const std::string& cell) {
    if (playerSocket == player1_socket)
        player1_selected_cells.push_back(cell);
    else if (playerSocket == player2_socket)
        player2_selected_cells.push_back(cell);
}

std::pair<bool, bool> Room::applyFire(int attackerSocket, const std::string& cell) {
    bool hit = false, sunk = false;
    auto& opponent_boats = (attackerSocket == player1_socket) ? player2_boats : player1_boats;
    auto& attacker_selected_cells = (attackerSocket == player1_socket) ? player1_selected_cells : player2_selected_cells;

    vector<string> boat_found;

    for (const auto& boat : opponent_boats) {
        for (const auto& part : boat) {
            if (cell == part) {
                hit = true;
                boat_found = boat;
                break;
            }
        }
        if (hit) break;
    }

    if (hit) {
        sunk = all_of(boat_found.begin(), boat_found.end(), [&](const string& part) {
            return find(attacker_selected_cells.begin(), attacker_selected_cells.end(), part) != attacker_selected_cells.end();
        });
    }

    logWithTimestamp("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)") + (sunk ? " (SUNK)" : ""));
    return {hit, sunk};
}

bool Room::checkVictory(int attackerSocket) {
    const auto& boats = (attackerSocket == player1_socket) ? player2_boats : player1_boats;
    const auto& selected = (attackerSocket == player1_socket) ? player2_selected_cells : player1_selected_cells;

    for (const auto& boat : boats) {
        for (const auto& coord : boat) {
            if (find(selected.begin(), selected.end(), coord) == selected.end()) return false;
        }
    }
    return true;
}

void Room::gameLoop() {
    logWithTimestamp("🎮 Iniciando bucle del juego...");
    logToFile("Bucle de juego iniciado.");

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(game_mutex);

        if (checkVictory(player1_socket)) {
            handleVictory(player1_socket);
            break;
        } else if (checkVictory(player2_socket)) {
            handleVictory(player2_socket);
            break;
        }
    }

    logWithTimestamp("🏁 Juego finalizado en la sala.");
    logToFile("Fin del bucle de juego.");
}

vector<vector<string>> Room::convertBoats(const vector<string>& data) {
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
