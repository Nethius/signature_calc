#include <iostream>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <future>
#include "safeQueue.h"
#include "safeMap.h"

uint32_t crc32Stream(uint32_t sum, std::shared_ptr<char> ptr, size_t count, uint32_t polynom)
{
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr.get());
    while (count--)
    {
        sum ^= *data++;
        for (size_t n = 0; n < 8; ++n)
            sum = (-(sum & 1) & polynom) ^ (sum >> 1);
    }
    return sum;
}

void calcCrcTask(SafeQueue& queue, const size_t blockSize, bool& isFileReaded, SafeMap& map, size_t threadNum)
{
    while(!isFileReaded || queue.size()) {
        std::shared_ptr<char> ptr = queue.frontAndPop();
        if (ptr.get()) {
            uint32_t value = crc32Stream(0xFFFFFFFF, ptr, blockSize, 0xEDB88320);
            map.insert(value);
            std::cout << value << " crc inserted, thread " << threadNum << std::endl;
        }
    }
}



void readFileTask(SafeQueue& queue, const size_t blockSize, bool& isFileReaded, std::string path)
{
    std::fstream fs;
    try{
        fs.open (path, std::fstream::in | std::fstream::binary);
    }
    catch (...) {
        std::cout << "cant open file " << path << std::endl;
        return;
    }
    fs.seekg (0, fs.end);
    size_t length = fs.tellg();
    fs.seekg (0, fs.beg);
    std::cout << "Reading " << length << " characters... " << std::endl;

    while (!fs.eof())
    {
        std::shared_ptr<char> bufPtr(new char[blockSize]);
        fs.read(bufPtr.get(), blockSize);
        queue.push(bufPtr);
    }
    isFileReaded = true;
    fs.close();
}

void writeFileTask(std::string path, SafeMap& map, bool& isCalcEnded)
{
    std::fstream fs;
    try {
        fs.open(path, std::fstream::out);
    }
    catch (...) {
        std::cout << "cant open file " << path << std::endl;
        return;
    }
    size_t index = 0;

    while(!isCalcEnded || map.size())
    {
        uint32_t value = map.getAndErase(index);
        if (value)
        {
            std::cout << value << " crc of chunk " << index << std::endl;
            fs << value << " ";
            index++;
        }
    }
    fs.close();
}

int main()
{
    std::cout << "Enter path for input file: " << std::endl;
    std::string inputFile;
    std::cin >> inputFile;

    std::cout << "Enter path for input file: " << std::endl;
    std::string outputFile;
    std::cin >> outputFile;

    std::cout << "Enter block size: " << std::endl;
    size_t blockSize;
    std::cin >> blockSize;

    std::cout << "Enter thread count: " << std::endl;
    size_t threadCount;
    std::cin >> threadCount;

    auto start = std::chrono::high_resolution_clock::now();

    SafeQueue queue;
    SafeMap map;
    bool isFileReaded = false;
    bool isCalcEnded = false;
    auto readRes = std::async(readFileTask, std::ref(queue), blockSize, std::ref(isFileReaded), inputFile);
    auto writeRes = std::async(writeFileTask, outputFile, std::ref(map), std::ref(isCalcEnded));
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < threadCount; i++)
        futures.push_back(std::async(calcCrcTask, std::ref(queue), blockSize, std::ref(isFileReaded), std::ref(map), i));

    for (size_t i = 0; i < threadCount; i++)
        futures[i].wait();
    isCalcEnded = true;

    writeRes.wait();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time from start: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
}
