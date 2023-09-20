#include "concurrent_queue.hh"

template <typename T>
ConcurrentQueue<T>::ConcurrentQueue()
{
}

template <typename T>
ConcurrentQueue<T>::~ConcurrentQueue()
{
}

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