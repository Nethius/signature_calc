#ifndef SIGNATURE_CALC_SAFEQUEUE_H
#define SIGNATURE_CALC_SAFEQUEUE_H
#include <queue>
#include <mutex>
#include <memory>
#include <map>



template <class T>
class SafeQueue
{
    typedef std::pair<size_t, T> pair_t;

    std::timed_mutex mutex;
    std::queue<pair_t> queue;
public:
    void push(T value, size_t index)
    {
        mutex.lock();
        queue.push(std::make_pair(index, value));
        mutex.unlock();
    }
    pair_t frontAndPop() {
        mutex.lock();
        pair_t hash;
        hash.second = 0;
        if (queue.size()) {
            hash = queue.front();
            queue.pop();
        }
        mutex.unlock();
        return hash;
    }
    size_t size()
    {
        mutex.lock();
        size_t size = queue.size();
        mutex.unlock();
        return size;
    }

};
#endif //SIGNATURE_CALC_SAFEQUEUE_H

