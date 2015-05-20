#ifndef PAEKDUSAN_KEEP_ALIVE_WORKER_H
#define PAEKDUSAN_KEEP_ALIVE_WORKER_H

#include <cstring>
#include "Utils/NetUtils.hpp"
#include "Utils/ThreadPool.hpp"
#include "HttpRequest.hpp"
#include "IHttpRequestHandler.hpp"

namespace Paekdusan {
    class KeepAliveWorker : public ITask {
    public:
        KeepAliveWorker(int sockfd, const IHttpRequestHandler& httpRequestHandler, int requestCount = 50, int timeout = 5)
            : _sockfd(sockfd), _httpRequestHandler(httpRequestHandler), _timeout(timeout), _requestCount(requestCount), _startTime(0) {}

    private:
        int _sockfd;
        int _timeout;
        int _requestCount;
        const IHttpRequestHandler& _httpRequestHandler;
        time_t _startTime;
        static const size_t RECV_BUFFER_SIZE = (1 << 14);
        char _recvBuff[RECV_BUFFER_SIZE];

    public:
        virtual void run() {
            do {
                if (!setSendTimeout(_sockfd, 1)) {
                    LogError("setSendTimeout failed: %d", getLastErrorNo());
                    break;
                }

                if (!setRecvTimeout(_sockfd, 1)) {
                    LogError("setRecvTimeout failed: %d", getLastErrorNo());
                    break;
                }
                HttpRequest httpRequest;
                int recvLen, parsedLength, unparsed = 0;

                while ((_startTime == 0 || time(nullptr) - _startTime <= _timeout) && _requestCount > 0) {
                    recvLen = recv(_sockfd, _recvBuff + unparsed, RECV_BUFFER_SIZE - unparsed, 0);
                    if (recvLen <= 0 && isTimeout()) {
                        LogError("recv returns %d", recvLen);
                        continue;
                    }
                    if (recvLen <= 0) {
                        LogError("recv failed: %d", getLastErrorNo());
                        break;
                    }

                    unparsed += recvLen;
                    parsedLength = httpRequest.parse(_recvBuff);

                    if (parsedLength < 0) {
                        LogError("parsed wrong http protocol format");
                        break;
                    }

                    if (parsedLength == 0) continue;

                    if (parsedLength <= unparsed) {
                        unparsed -= parsedLength;
                        memmove(_recvBuff, _recvBuff + parsedLength, unparsed);
                    }

                    if (httpRequest.readBodyFinished()) {
                        if (_startTime == 0) _startTime = time(nullptr);
                        
                        string connection;
                        if (httpRequest.getHeaderByKey("CONNECTION", connection) && connection == "close") _requestCount = 1;

                        _requestCount--;

                        string response = _httpRequestHandler.handle(httpRequest);
                        if (!_send(response)) {
                            LogError("send http response failed");
                            break;
                        }

                        httpRequest.reset();
                    }
                }
            } while (false);

            if (closeSocket(_sockfd) < 0) {
                LogError("close socket failed: %d", getLastErrorNo());
            }
        }
    private:
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
    };
}

#endif