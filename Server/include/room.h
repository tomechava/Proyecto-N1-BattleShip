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

#include <string>
#include <map>
#include <mutex>
#include <vector>
#include "protocol.h"

class Room {
public:
    Room(int player1_socket, int player2_socket);
    void run();
    void onPlayerMessage(int playerSocket, const ProtocolMessage& msg);

private:
    int player1_socket;
    int player2_socket;

    int current_turn_socket;
    int other_player_socket;

    std::mutex game_mutex;

    bool player1_ready = false;
    bool player2_ready = false;

    // Tableros de cada jugador: casilla -> bool (true = barco presente)
    std::map<std::string, bool> player1_board;
    std::map<std::string, bool> player2_board;

    void waitForPlayersReady();
    void gameLoop();

    // Métodos para manejar distintos tipos de mensajes
    void handlePlace(int playerSocket, const ProtocolMessage& msg);
    void handleReady(int playerSocket);
    void handleFire(int playerSocket, const ProtocolMessage& msg);
    void handleChat(int senderSocket, const ProtocolMessage& msg);
    void handleVictory(int winnerSocket);

    // Auxiliares
    bool applyFire(int attackerSocket, const std::string& cell);
    bool checkVictory(int attackerSocket);
};
