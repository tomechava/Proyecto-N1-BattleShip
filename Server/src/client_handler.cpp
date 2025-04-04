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
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "Cliente desconectado." << endl;
            break;
        }

        string message(buffer);
        MessageType type = stringToMessageType(message);     //obtiene el tipo de mensaje

        // Aqui decide la respuesta con base en el protocolo
        string response;
        switch (type) {
            case MessageType::REGISTER:
                response = "REGISTRO_OK";
                break;
            case MessageType::READY:
                response = "LISTO_OK";
                break;
            case MessageType::FIRE:
                response = "DISPARO_OK";
                break;
            case MessageType::HIT:
                response = "GOLPEADO_OK";
                break;
            case MessageType::MISS:
                response = "FALLADO_OK";
                break;
            case MessageType::SUNK:
                response = "HUNDIDO_OK";
                break;
            case MessageType::WIN:
                response = "GANASTE_OK";
                break;
            case MessageType::LOSE:
                response = "PERDISTE_OK";
                break;
            case MessageType::CHAT:
                response = "CHAT_OK";
                break;
            case MessageType::DISCONNECT:
                response = "DESCONECTADO_OK";
                break;
            default:
                response = "ERROR_COMANDO";
                break;
        }

        send(clientSocket, response.c_str(), response.size(), 0);
    }

    CLOSE_SOCKET(clientSocket);

}
