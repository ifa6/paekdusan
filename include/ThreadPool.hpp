#ifndef PAEKDUSAN_THREAD_POOL_H
#define PAEKDUSAN_THREAD_POOL_H

#include <thread>
#include "Queue.hpp"

namespace Paekdusan {
    class ITask {
    public:
        virtual void run() = 0;
        virtual ~ITask() {}
    };

    class ThreadPool {
    public:
        ThreadPool(size_t threadNum, size_t queueCapacity) : _taskQueue(queueCapacity) {
            _threads.reserve(threadNum);
            for (size_t i = 0; i < threadNum; i++) {
                _threads.push_back(thread(&ThreadPool::worker, this));
            }
        }

        ~ThreadPool() {
            for (size_t i = 0; i < _threads.size(); i++) {
                _taskQueue.push(nullptr);
            }
            for (size_t i = 0; i < _threads.size(); i++) {
                _threads[i].join();
            }
        }

        void add(ITask* task) {
            assert(task != nullptr);
            _taskQueue.push(task);
        }

    private:
        void worker() {
            while (true) {
                ITask* task = _taskQueue.pop();
                if (task == nullptr) break;

                task->run();
                delete task;
            }
        }

        vector<thread> _threads;
        BoundedBlockingQueue<ITask*> _taskQueue;

        ThreadPool(const ThreadPool&) = delete;
        const ThreadPool& operator =(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        const ThreadPool& operator =(ThreadPool&&) = delete;
    };
}

#endif