#include "../include/client_handler.h"
#include "../include/protocol.h"
#include "../include/utils.h"
#include <iostream>
#include <thread>
#include <string>
#include <vector>

using namespace std;

void handleClientMessages(int clientSocket) {
    char buffer[1024];
    int bytesReceived;

    while (true) {
        memset(buffer, 0, sizeof(buffer));  // Limpiar el buffer antes de recibir un nuevo mensaje
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);  // Recibir mensaje del cliente

        if (bytesReceived <= 0) {       // Si no se recibe nada o hay un error, el cliente se desconecta
            logWithTimestamp("Cliente desconectado.");
            break;
        }

        string rawMessage(buffer);
        ProtocolMessage msg = parseMessage(rawMessage);

       if (msg.type == MessageType::UNKNOWN) {  // Si el mensaje no es válido, enviamos un error
            string errorMsg = createMessage(MessageType::UNKNOWN, { "Comando no válido" });
            send(clientSocket, errorMsg.c_str(), errorMsg.size(), 0);       // Enviar mensaje de error a cliente
            continue;
        }

        // Pasar el mensaje a la sala (Room) para que decida qué hacer
        room->onPlayerMessage(clientSocket, msg);
    }

    CLOSE_SOCKET(clientSocket);

}
