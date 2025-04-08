// room.h
#ifndef ROOM_H
#define ROOM_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>

struct Room {
public:
    int player1_socket;     // Socket del jugador 1
    int player2_socket;     // Socket del jugador 2
    std::thread game_thread;        // Hilo del juego
    bool game_running;     // Indica si el juego está en curso

    std::atomic<bool> player1_ready;
    std::atomic<bool> player2_ready;

    Room(int p1, int p2);
    void run();

    // Callback que client_handler usará para enviar los mensajes a la Room
    void onPlayerMessage(int playerSocket, const ProtocolMessage& msg);
    
private:
    void waitForPlayersReady();
    void gameLoop();

    int current_turn_socket;
    int other_player_socket;
    std::mutex game_mutex;
};

#endif // ROOM_H
