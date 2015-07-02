#ifndef SOCKETS_H_U0SPYFVF
#define SOCKETS_H_U0SPYFVF

//sockets include for windows
#if defined(_WIN32)
#include <winsock2.h>

#ifndef __MINGW32__
#pragma comment(lib, "ws2_32.lib")
#endif

#include <WS2tcpip.h>

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <iostream>
int createSocket();
void bindSocket(int sock,const sockaddr_in &addr);

#endif /* end of include guard: SOCKETS_H_U0SPYFVF */
