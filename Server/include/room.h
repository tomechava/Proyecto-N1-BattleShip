#pragma once   

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

#include <cstring>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "protocol.h"
#include <vector>
#include <set>
#include <sstream>

using namespace std;

class Room {
public:
    Room(int player1_socket, int player2_socket);
    void run();
    void onPlayerMessage(int playerSocket, const ProtocolMessage& msg);
    void addSelectedCell(int playersocket, const std::string& cell);
    

private:
    int player1_socket;
    int player2_socket;

    int current_turn_socket;
    int other_player_socket;

    std::mutex game_mutex;

    bool player1_ready = false;
    bool player2_ready = false;

    // Listado de botes de cada jugador
    vector<set<string>> player1_boats;
    vector<set<string>> player2_boats;

    void waitForPlayersReady();
    void gameLoop();

    // Métodos para manejar distintos tipos de mensajes
    
    void handleReady(int playerSocket, const ProtocolMessage& msg);	
    void handleFire(int playerSocket, const ProtocolMessage& msg);
    void handleChat(int senderSocket, const ProtocolMessage& msg);
    void handleVictory(int winnerSocket);

    // Auxiliares
    std::pair<bool, bool> applyFire(int attackerSocket, const std::string& cell);
    bool checkVictory(int attackerSocket);
    vector<set<string>> convertBoats(const string& data);

    // Posicionar seleccion de casillas de cada jugador en 1 arreglo
    vector<std::string> player1_selected_cells;
    vector<std::string> player2_selected_cells;
};
