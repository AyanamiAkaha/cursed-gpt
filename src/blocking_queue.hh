#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <type_traits>

template <typename T>
class BlockingQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable cond;
public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;
    static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");

    void push(const T& v);
    T pop();
    bool empty();
    void operator<<(const T& v);
    T operator>>(T& v);
};

template <typename T>
void BlockingQueue<T>::push(const T& v)
{
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(T(v));
    cond.notify_one();
}

template <typename T>
T BlockingQueue<T>::pop()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.empty()) {
        cond.wait(lock, [this]() { return !queue.empty(); });
    }
    T v = queue.front();
    queue.pop();
    return T(v);
}

template <typename T>
bool BlockingQueue<T>::empty()
{
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
}

template <typename T>
void BlockingQueue<T>::operator<<(const T& v) {
    push(v);
}

template <typename T>
T BlockingQueue<T>::operator>>(T& v) {
    return pop();
}