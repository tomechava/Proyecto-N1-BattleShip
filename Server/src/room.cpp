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
    string mensajeJugador = "Bienvenido. Esperando a que coloques tus barcos.\n";

    //Enviar REGISTER y mensajeJugador a cada jugador
    ProtocolMessage msg = { MessageType::REGISTER, {} };

    msg.data.push_back(mensajeJugador);
    string room_joined_msg = createMessage(MessageType::REGISTER, msg.data);

    // Enviar mensajes a los jugadores
    ssize_t sent1 = send(player1_socket, room_joined_msg.c_str(), room_joined_msg.size(), 0);
    if (sent1 == -1) {
        perror("Error al enviar mensaje al jugador 1");
    }
    ssize_t sent2 = send(player2_socket, room_joined_msg.c_str(), room_joined_msg.size(), 0);
    if (sent2 == -1) {
        perror("Error al enviar mensaje al jugador 2");
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
    }
    ssize_t sent_start2 = send(player2_socket, start_message.c_str(), start_message.size(), 0);
    if (sent_start2 == -1) {
        perror("Error al enviar mensaje de inicio al jugador 2");
    }

    logWithTimestamp("Ambos jugadores listos. ¡Comienza el juego!");

    current_turn_socket = player2_socket; // por ejemplo, el player 2 comienza
    other_player_socket = player1_socket;

    // Enviar mensaje TURN al jugador que comienza
    ProtocolMessage turn_msg = { MessageType::TURN, {"Es tu turno."} };
    string msg_str = createMessage(turn_msg.type, turn_msg.data);
    ssize_t sent_turn = send(current_turn_socket, msg_str.c_str(), msg_str.size(), 0);
    if (sent_turn == -1) {
        perror("Error al enviar mensaje de turno al jugador 1");
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
    std::lock_guard<std::mutex> lock(game_mutex);

    switch (msg.type) {
        case MessageType::READY:
            handleReady(playerSocket, msg);
            break;
        case MessageType::FIRE:
            handleFire(playerSocket, msg);
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
    logWithTimestamp("Barcos recibidos (" + to_string((playerSocket == player1_socket ? 1 : 2)) + "):");
    playerSocket == player1_socket ? cout << player1_boats : cout << player2_boats;
    cout << endl;
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
    addSelectedCell(playerSocket, cell);  // Agregar la celda disparada a la lista de celdas seleccionadas
    auto [hit, sunk] = applyFire(playerSocket, cell);  // Destructuración válida

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


// Metodo para agregar las casillas jugadas a un arreglo (individual por cada player)
void Room::addSelectedCell(int playerSocket, const std::string& cell){
    if (playerSocket == player1_socket){
        player1_selected_cells.push_back(cell);
    }else if (playerSocket == player2_socket){
        player2_selected_cells.push_back(cell);
    }
    
}


std::pair<bool, bool> Room::applyFire(int attackerSocket, const std::string& cell) {
    bool hit = false;
    bool sunk = false;

    auto& opponent_boats = (attackerSocket == player1_socket) ? player2_boats : player1_boats;
    auto& attacker_selected_cells = (attackerSocket == player1_socket) ? player1_selected_cells : player2_selected_cells;

    vector<string> boat_found;

    for (const auto& boat : opponent_boats) {
        for (const auto& boat_part : boat) {
            if (cell == boat_part) {
                hit = true;
                boat_found = boat;
                break;
            }
        }
        if (hit) break;
    }

    if (hit) {

        sunk = true;
        for (const auto& boat_part : boat_found) {
            if (find(attacker_selected_cells.begin(), attacker_selected_cells.end(), boat_part) == attacker_selected_cells.end()) {
                sunk = false;
                break;
            }
        }
    }

    logWithTimestamp("Jugador disparó a " + cell + (hit ? " (HIT)" : " (MISS)") + (sunk ? " (SUNK)" : ""));
    return {hit, sunk};
}



// Verificar si el jugador ha ganado
bool Room::checkVictory(int attackerSocket) {
    const auto& boats = (attackerSocket == player1_socket) ? player2_boats : player1_boats;
    const auto& selected_cells = (attackerSocket == player1_socket) ? player2_selected_cells : player1_selected_cells;
    for (const auto& boat : boats) {
        for (const auto& coord : boat) {
            if (find(selected_cells.begin(), selected_cells.end(), coord) == selected_cells.end()) {
                return false; // Coordenada no encontrada
            }
        }
    }
    return true;
}

void Room::gameLoop() {
    logWithTimestamp("🎮 Iniciando bucle del juego...");

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
}

//Transformar msg de botes a arreglo bidimensional
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