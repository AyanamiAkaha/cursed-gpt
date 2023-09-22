#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <type_traits>

template <typename T>
class ConcurrentQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;
    static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");

    void push(const T& v);
    T pop();
    bool empty();
    void operator<<(const T& v);
    T operator>>(T& v);
};

template <typename T>
void ConcurrentQueue<T>::push(const T& v)
{
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(T(v));
}

template <typename T>
T ConcurrentQueue<T>::pop()
{
    std::lock_guard<std::mutex> lock(mutex);
    T v = queue.front();
    queue.pop();
    return T(v);
}

template <typename T>
bool ConcurrentQueue<T>::empty()
{
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
}

template <typename T>
void ConcurrentQueue<T>::operator<<(const T& v) {
    push(v);
}

template <typename T>
T ConcurrentQueue<T>::operator>>(T& v) {
    return pop();
}