#include "../include/protocol.h"
#include <algorithm>  //Por si toca manipular strings(no seguros aun)

using namespace std; //para evitar repetir  en cada uso

//Toma un string y lo convierte al enum correspondiente
MessageType stringToMessageType(const string& str) {
    if (str == "REGISTER") return MessageType::REGISTER;
    if (str == "READY") return MessageType::READY;
    if (str == "TURN") return MessageType::TURN;
    if (str == "FIRE") return MessageType::FIRE;
    if (str == "HIT") return MessageType::HIT;
    if (str == "MISS") return MessageType::MISS;
    if (str == "SUNK") return MessageType::SUNK;
    if (str == "WIN") return MessageType::WIN;
    if (str == "LOSE") return MessageType::LOSE;
    if (str == "DISCONNECT") return MessageType::DISCONNECT;
    return MessageType::UNKNOWN;
}

//lo inverso: recibe un MessageType y devuelve el string
string messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::REGISTER: return "REGISTER";
        case MessageType::READY: return "READY";
        case MessageType::TURN: return "TURN";
        case MessageType::FIRE: return "FIRE";
        case MessageType::HIT: return "HIT";
        case MessageType::MISS: return "MISS";
        case MessageType::SUNK: return "SUNK";
        case MessageType::WIN: return "WIN";
        case MessageType::LOSE: return "LOSE";
        case MessageType::DISCONNECT: return "DISCONNECT";
        default: return "UNKNOWN";
    }
}

//Convierte un string crudo como "FIRE|B4" en una estructura ProtocolMessage
ProtocolMessage parseMessage(const string& raw) {
    ProtocolMessage result;
    stringstream ss(raw);
    string typeStr, rest;

    // extrae lo que está antes del primer '|'
    if (getline(ss, typeStr, '|')) {
        result.type = stringToMessageType(typeStr);
    } else {
        result.type = MessageType::UNKNOWN;
        return result;
    }

    // el resto del mensaje puede tener múltiples datos separados por comas
    while (getline(ss, rest, ',')) {
        result.data.push_back(rest);
    }

    return result;
}


//Hace lo contrario a parseMessage recibe el protoclMessage y lo convierte a un string
string createMessage(MessageType type, const vector<string>& data) {
    string message = messageTypeToString(type) + "|";

    for (size_t i = 0; i < data.size(); ++i) {
        message += data[i];
        if (i < data.size() - 1) message += ",";
    }
    
    return message;
}


