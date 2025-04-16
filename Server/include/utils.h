#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
#endif

#include <string>
#include <vector>
#include <set>

void logWithTimestamp(const std::string& msg);

// Log al archivo por defecto
void logToFile(const std::string& message);

// Log a archivo específico
void logToFile(const std::string& file, const std::string& message);

std::string vectorToString(const std::vector<std::string>& vec);
std::string vectorOfVectorsToString(const std::vector<std::vector<std::string>>& vec);
std::string setToString(const std::set<std::string>& s);
std::string vectorOfSetsToString(const std::vector<std::set<std::string>>& vec);

// Macros para logs separados
#define LOG_SERVER(msg) logToFile("logs/server.log", msg)
#define LOG_ROOM(msg)   logToFile("logs/room.log", msg)

