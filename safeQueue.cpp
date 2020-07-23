#include "safeQueue.h"
#include <chrono>

void SafeQueue::push(std::shared_ptr<char> ptr)
{
    while(!mutex.try_lock())
        ;
    queue.push(ptr);
    mutex.unlock();
}
std::shared_ptr<char> SafeQueue::frontAndPop()
{
    while(!mutex.try_lock())
        ;
    std::shared_ptr<char> ptr = nullptr;
    if (queue.size())
    {
        ptr = queue.front();
        queue.pop();
    }
    mutex.unlock();
    return ptr;
}

size_t SafeQueue::size()
{
    while(!mutex.try_lock())
        ;
    size_t size = queue.size();
    mutex.unlock();
    return size;
}