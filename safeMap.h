#ifndef SIGNATURE_CALC_SAFEMAP_H
#define SIGNATURE_CALC_SAFEMAP_H
#include <map>
#include <mutex>
#include <memory>

class SafeMap
{
    std::timed_mutex mutex;
    std::map<size_t, uint32_t> map;
    size_t index = 0;

public:
    void insert(uint32_t value);
    uint32_t getAndErase(size_t index);
    size_t size();

};
#endif //SIGNATURE_CALC_SAFEMAP_H
