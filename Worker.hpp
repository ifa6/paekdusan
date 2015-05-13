#ifndef PAEKDUSAN_WORKER_H
#define PAEKDUSAN_WORKER_H

#include <cstring>
#include "Utils/NetUtils.hpp"
#include "Utils/ThreadPool.hpp"
#include "HttpRequest.hpp"
#include "IHttpRequestHandler.hpp"

namespace Paekdusan {
    class Worker : public ITask {
    public:
        Worker(int sockfd, const IHttpRequestHandler& httpRequestHandler) 
            : _sockfd(sockfd), _httpRequestHandler(httpRequestHandler) {}

    private:
        int _sockfd;
        static const size_t RECV_BUFFER_SIZE = (1 << 14);
        const IHttpRequestHandler& _httpRequestHandler;

        bool _receive(HttpRequest& httpRequest) {
            char recvBuff[RECV_BUFFER_SIZE];
            int recvLen = 0, parsedLength = 0, unparsed = 0;
            
            while (!httpRequest.readBodyFinished()) {
                recvLen = recv(_sockfd, recvBuff + unparsed, RECV_BUFFER_SIZE - unparsed, 0);
                if (recvLen <= 0) {
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
                
                /*parsed part or all*/
                if (parsedLength <= unparsed) {
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
                if (sentLen == 0) LogInfo("send returns 0");

                alreadySent += sentLen;
            }
            return true;
        }

    public:
        virtual void run() {
            do {
                if (!setSendTimeout(_sockfd, 5)) {
                    LogError("setSendTimeout failed: %d", getLastErrorNo());
                    break;
                }

                if (!setRecvTimeout(_sockfd, 5)) {
                    LogError("setRecvTimeout failed: %d", getLastErrorNo());
                    break;
                }

                HttpRequest httpRequest;
                if (!_receive(httpRequest)) {
                    LogError("receive failed");
                    break;
                }

                string response = _httpRequestHandler.handle(httpRequest);

                if (!_send(response)) {
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
