#ifndef PAEKDUSAN_THREADPOOL_SERVER_H
#define PAEKDUSAN_THREADPOOL_SERVER_H

#include "ThreadPool.hpp"
#include "HttpProcessTask.hpp"

namespace Paekdusan {
    class ThreadPoolServer {
    public:
        ThreadPoolServer(size_t threadNum, size_t taskQueueCapacity, int listenQueueLen, size_t port) :
            _threadPool(threadNum, taskQueueCapacity), _hostSocket(-1), _listenQueueLen(listenQueueLen) {
            _hostSocket = createAndListenSocket(port, _listenQueueLen);
            assert(_hostSocket > 0);
        }

        bool start() {
            sockaddr_in clientAddr;
            int nSize = sizeof(clientAddr);
            int clientSock;

            while (true) {
                if (-1 == (clientSock = acceptClient(_hostSocket, (struct sockaddr*) &clientAddr, &nSize))) {
                    LogError("accept failed: %d, %s", getLastErrorNo(), strerror(getLastErrorNo()));
                    break;
                }
                LogInfo("accept client: %s", inet_ntoa(clientAddr.sin_addr));
                _threadPool.add(new HttpProcessTask(clientSock));
            }
            return true;
        }

    private:
        ThreadPool _threadPool;
        int _hostSocket;
        const int _listenQueueLen;

        ThreadPoolServer(const ThreadPoolServer&) = delete;
        const ThreadPoolServer& operator =(const ThreadPoolServer&) = delete;
        ThreadPoolServer(ThreadPoolServer&&) = delete;
        const ThreadPoolServer& operator =(ThreadPoolServer&&) = delete;
    };
}

#endif