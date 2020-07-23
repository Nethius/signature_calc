#include "safeMap.h"
#include <chrono>

void SafeMap::insert(uint32_t value)
{
    while(!mutex.try_lock())
        ;
    map.insert(std::pair<size_t, uint32_t>(index, value));
    index++;
    mutex.unlock();
}

uint32_t SafeMap::getAndErase(size_t index)
{
    while(!mutex.try_lock())
        ;
    try
    {
        uint32_t value = map.at(index);
        map.erase(index);
        mutex.unlock();
        return value;
    }
    catch (...) {
        mutex.unlock();
        return 0;
    }
}


size_t SafeMap::size()
{
    while(!mutex.try_lock())
        ;
    size_t size = map.size();
    mutex.unlock();
    return size;
}