#ifndef SIGNATURE_CALC_SAFEQUEUE_H
#define SIGNATURE_CALC_SAFEQUEUE_H
#include <queue>
#include <mutex>
#include <memory>

class SafeQueue
{
    std::timed_mutex mutex;
    std::queue<std::shared_ptr<char>> queue;

public:
    void push(std::shared_ptr<char> ptr);
    std::shared_ptr<char> frontAndPop();
    size_t size();

};
#endif //SIGNATURE_CALC_SAFEQUEUE_H

