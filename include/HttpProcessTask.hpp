#ifndef PAEKDUSAN_HTTP_PROCESS_TASK_H
#define PAEKDUSAN_HTTP_PROCESS_TASK_H

#include <cstring>
#include "NetTools.hpp"
#include "HttpRequest.hpp"
#include "ThreadPool.hpp"

namespace Paekdusan {
    const size_t RECV_BUFFER_SIZE = (2 << 14);

    const char* const HTTP_RESPONSE = "HTTP/1.1 %d OK\r\nConnection: close\r\nServer: Paekdusan/1.0.0\r\nContent-Type: text/json; charset=UTF-8\r\nContent-Length: %d\r\n\r\n%s";

    class HttpProcessTask : public ITask {
    public:
        HttpProcessTask(int sockfd) : _sockfd(sockfd) {}
    private:
        int _sockfd;
        bool _receive(HttpRequest& httpRequest) {
            char recvBuff[RECV_BUFFER_SIZE];
            int recvLen = 0, parsedLength = 0, unparsed = 0;
            
            while (!httpRequest.readBodyFinished()) {
                recvLen = recv(_sockfd, recvBuff + unparsed, RECV_BUFFER_SIZE - unparsed, 0);
                if (recvLen < 0) {
                    LogError("recv failed: %d", getLastErrorNo());
                    return false;
                }

                unparsed += recvLen;
                parsedLength = httpRequest.parse(recvBuff);

                /*error occurred*/
                if (parsedLength < 0) {
                    LogError("parsed wrong http protocol format");
                    return false;
                }

                /*more data need to be read before being parsed*/
                if (parsedLength == 0) continue;
                
                /*parsed part*/
                if (parsedLength < unparsed) {
                    unparsed -= parsedLength;
                    memmove(recvBuff, recvBuff + parsedLength, unparsed);
                }
            }
            return true;
        }

        bool _send(const string& dataSend) {
            int sentLen = 0;
            size_t alreadySent = 0, dataLen = dataSend.length();
            while (alreadySent < dataSend.length()) {
                sentLen = send(_sockfd, dataSend.c_str() + alreadySent, dataLen - alreadySent, 0);
                if (sentLen < 0) {
                    LogError("send failed: %d", getLastErrorNo());
                    return false;
                }
                alreadySent += sentLen;
            }
            return true;
        }

        bool _setsockopt() {
            struct linger LNG = {1, 1};
            struct timeval SOCKET_TIMEOUT = {16, 0};

            if (-1 == setsockopt(_sockfd, SOL_SOCKET, SO_LINGER, (const char*) &LNG, sizeof(LNG))) {
                LogError("setsockopt failed: %d", getLastErrorNo());
                return false;
            }
            if (-1 == setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*) &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT))) {
                LogError("setsockopt failed: %d", getLastErrorNo());
                return false;
            }
            if (-1 == setsockopt(_sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*) &SOCKET_TIMEOUT, sizeof(SOCKET_TIMEOUT))) {
                LogError("setsockopt failed: %d", getLastErrorNo());
                return false;
            }
            return true;
        }
    public:
        void run() {
            do {
                if (!_setsockopt()) {
                    LogError("set socket property failed");
                    break;
                }

                HttpRequest httpRequest;
                if (!_receive(httpRequest)) {
                    LogError("receive failed");
                    break;
                }

                string response = "{'hello': 'world'}";
                if (!_send(stringFormat(HTTP_RESPONSE, 200, response.length(), response.c_str()))) {
                    LogError("send http response failed");
                    break;
                }
            } while (false);


            if (closeSocket(_sockfd) < 0) {
                LogError("close socket failed: %d", getLastErrorNo());
            }
        }
    };
}

#endif