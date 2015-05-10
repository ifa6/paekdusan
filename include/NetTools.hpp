#ifndef PAEKDUSAN_NET_TOOLS_H
#define PAEKDUSAN_NET_TOOLS_H

//headers in windows 
#if defined(_WIN32)
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
//headers in linux
#elif defined(__linux__)
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string>
#include "Logger.hpp"

namespace Paekdusan {
    using namespace std;

    int getLastErrorNo() {
#if defined(_WIN32)
        return GetLastError();
#elif defined(__linux__)
        return errno;
#endif
    }

    int closeSocket(int sock) {
#if defined(_WIN32)
        return closesocket(sock);
#elif defined(__linux__)
        return close(sock);
#endif
    }

    int acceptClient(int sock, struct sockaddr* clientAddr, void* addrLen) {
#if defined(_WIN32)
        return accept(sock, clientAddr, (int*) addrLen);
#elif defined(__linux__)
        return accept(sock, clientAddr, (socklen_t*) addrLen);
#endif
    }

    int createAndListenSocket(int port, int listenQueueLength) {
#if defined(_WIN32)
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) < 0) {
            LogError("WSAStartup failed: %d", getLastErrorNo());
            return -1;
        }
#endif

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            LogError("create socket failed: %d, %s", getLastErrorNo(), strerror(getLastErrorNo()));
            return -1;
        }

        int optval = 1; // nozero
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) (&optval), sizeof(optval)) < 0) {
            LogError("setsockopt failed: %d, %s", getLastErrorNo(), strerror(getLastErrorNo()));
            return -1;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) < 0) {
            LogError("::bind failed: %d, %s", getLastErrorNo(), strerror(getLastErrorNo()));
            return -1;
        }

        if (listen(sock, listenQueueLength) < 0) {
            LogError("listen failed: %d, %s", getLastErrorNo(), strerror(getLastErrorNo()));
            return -1;
        }

        return sock;
    }

}


#endif 