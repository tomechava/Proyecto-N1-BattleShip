//evitar que el archivo sea incluido más de una vez en la compilacion
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>       //para manejar cadenas de texto (string)
#include <vector>       //para listas dinámicas (vector) para los datos del mensaje
#include <sstream>      //para dividir mensajes usando delimitadores (stringstream)

using namespace std; //para evitar repetir  en cada uso

enum class MessageType {    //define los tipos de mensajes que se pueden enviar
    REGISTER,
    READY,
    FIRE,
    HIT,
    MISS,
    SUNK,
    WIN,
    LOSE,
    CHAT,
    DISCONNECT,
    PLACE,
    UNKNOWN
};


struct ProtocolMessage {    //estructura para definir un mensaje de protocolo   
    MessageType type;
    vector<string> data;      //lista de datos del mensaje: type (tipo de mensaje), data (datos del mensaje)
};


//Funciones para convertir entre MessageType, string y ProtocolMessage

//convierte un string como "REGISTER" en MessageType::REGISTER
MessageType stringToMessageType(const string& str);    

//inverso: convierte el enum en string
string messageTypeToString(MessageType type); 

//interpreta un mensaje crudo como "FIRE|B4" y lo vuelve en un ProtocolMessage
ProtocolMessage parseMessage(const string& raw);   

//genera un string a partir de un MessageType y sus datos
string createMessage(MessageType type, const vector<string>& data);      

#endif
