#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "room.h" // Incluir la definición de la clase Room

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

class Room; // Declaración anticipada de la clase Room
// Esto es necesario para evitar dependencias circulares

void handleClientMessages(int clientSocket, Room* room); // Prototipo de la función que maneja los mensajes del cliente
// Esta función se encargará de recibir mensajes del cliente y procesarlos

#endif