#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

void handleClientMessages(int clientSocket);

#endif