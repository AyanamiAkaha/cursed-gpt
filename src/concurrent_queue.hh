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
    ConcurrentQueue();
    ~ConcurrentQueue();
    static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");

    void push(const T& v);
    T pop();
    bool empty();
    void operator<<(const T& v);
    T operator>>(T& v);
};