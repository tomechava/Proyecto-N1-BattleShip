// room.h
#ifndef ROOM_H
#define ROOM_H

#include <thread>

struct Room {
    int player1_socket;     // Socket del jugador 1
    int player2_socket;     // Socket del jugador 2
    std::thread game_thread;        // Hilo del juego
    bool game_running;     // Indica si el juego está en curso

    Room(int p1, int p2);
    void startGame();
    void handleGame(); // Lógica del juego
    
};

#endif // ROOM_H
