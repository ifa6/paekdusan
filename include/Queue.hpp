#ifndef PAEKDUSAN_QUEUE_H
#define PAEKDUSAN_QUEUE_H

#include <vector> 
#include <queue>
#include <mutex>
#include <cassert>
#include <condition_variable>

namespace Paekdusan {
    using namespace std;

    template<typename T>
    class BoundQueue {
    public:
        void push(const T& t) {
            assert(!full());
            _buff[_tail] = t;
            _tail = (_tail + 1 % _capacity);
            _size++;
        }

        T pop() {
            assert(!empty());
            size_t oldHead = _head;
            _head = (_head + 1) % _capacity;
            _size--;
            return _buff[oldHead];
        }

        void clear() {
            _head = 0;
            _tail = 0; 
            _size = 0;
        }
        bool empty() const {
            return _size == 0;
        }

        bool full() const {
            return _capacity == _size;
        }

        size_t size() const {
            return _size;
        }

        size_t capacity() const {
            return _capacity;
        }

        explicit BoundQueue(size_t capacity) :
            _capacity(capacity), _buff(capacity),
            _head(0), _tail(0), _size(0) {}

        explicit BoundQueue(const BoundQueue& other) :
            _head(other._head), _tail(other._tail),
            _size(other._size), _capacity(other._capacity),
            _buff(other._buff) {}

        explicit BoundQueue(BoundQueue&& other) :
            _head(other._head), _tail(other._tail),
            _size(other._size), _capacity(other._capacity),
            _buff(move(other._buff)) {}

        BoundQueue& operator= (BoundQueue&& other) {
            assert(_capacity == other._capacity);

            _head = other._head;
            _tail = other._tail;
            _size = other._size;
            _buff = move(other._buff);

            return *this;
        }

        BoundQueue& operator= (const BoundQueue& other) {
            assert(_capacity == other._capacity);

            _head = other._head;
            _tail = other._tail;
            _size = other._size;
            _buff = other._buff;

            return *this;
        }
    private:
        size_t _head;
        size_t _tail;
        size_t _size;
        const size_t _capacity;
        vector<T> _buff;
    };

    template<class T>
    class BlockingQueue {
    public:
        BlockingQueue() {}
        void push(const T& x) {
            unique_lock<mutex> lock(_mutex);
            _queue.push(x);
            _notEmptyCond.notify_one();
        }

        void pop() {
            unique_lock<mutex> lock(_mutex);
            while (_queue.size() == 0) {
                _notEmptyCond.wait(lock);
            }
            _queue.pop();
        }

        size_t size() const {
            unique_lock<mutex> lock(_mutex);
            return _queue.size();
        }

        bool empty() const {
            unique_lock<mutex> lock(_mutex);
            return _queue.size() == 0;
        }

    private:
        queue<T> _queue;
        mutable mutex _mutex;
        condition_variable _notEmptyCond;

        BlockingQueue(const BlockingQueue&) = delete;
        const BlockingQueue& operator =(const BlockingQueue&) = delete;
        BlockingQueue(BlockingQueue&&) = delete;
        const BlockingQueue& operator =(BlockingQueue&&) = delete;
    };

    template<typename T>
    class BoundedBlockingQueue {
    public:
        explicit BoundedBlockingQueue(size_t capacity) : _capacity(capacity), _buff(capacity), _head(0), _tail(0), _size(0) {}
        
        void clear() {
            unique_lock<mutex> lock(_mutex);
            _head = 0;
            _tail = 0;
            _size = 0;
        }
        
        bool empty() const {
            unique_lock<mutex> lock(_mutex);
            return _size == 0;
        }
        
        bool full() const {
            unique_lock<mutex> lock(_mutex);
            return _capacity == _size;
        }
        
        size_t size() const {
            unique_lock<mutex> lock(_mutex);
            return _size;
        }
        
        size_t capacity() const {
            return _capacity;
        }

        void push(const T& t) {
            unique_lock<mutex> lock(_mutex);
            while (_capacity == _size) {
                _notFullCond.wait(lock);
            }
            _buff[_tail] = t;
            _tail = (_tail + 1) % _capacity;
            _size++;
            _notEmptyCond.notify_one();
        }

        T pop() {
            unique_lock<mutex> lock(_mutex);
            while (_size == 0) {
                _notEmptyCond.wait(lock);
            }
            size_t oldHead = _head;
            _head = (_head + 1) % _capacity;
            _size--;
            _notFullCond.notify_one();
            return _buff[oldHead];
        }

    private:
        size_t _head;
        size_t _tail;
        size_t _size;
        const size_t _capacity;
        vector<T> _buff;
        mutable mutex _mutex;
        condition_variable _notEmptyCond;
        condition_variable _notFullCond;

        BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;
        const BoundedBlockingQueue& operator =(const BoundedBlockingQueue&) = delete;
        BoundedBlockingQueue(BoundedBlockingQueue&&) = delete;
        const BoundedBlockingQueue& operator =(BoundedBlockingQueue&&) = delete;
    };
}

#endif