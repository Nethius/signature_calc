#include <iostream>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <future>
#include "safeQueue.h"
#include <cstring>
#include <filesystem>

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

void calcCrcTask(SafeQueue<std::shared_ptr<char>>& queue, const size_t blockSize, bool& isFileReaded, SafeQueue<uint32_t>& writeQueue, size_t threadNum)
{
    while(!isFileReaded || queue.size()) {
        std::pair<size_t, std::shared_ptr<char>> dataBlock = queue.frontAndPop();
        if (dataBlock.second.get()) {
            uint32_t value = crc32Stream(0xFFFFFFFF, dataBlock.second, blockSize, 0xEDB88320);
            writeQueue.push(value, dataBlock.first);
            std::cout << value << " crc inserted, thread " << threadNum << std::endl;
        }
    }
}

void readFileTask(SafeQueue<std::shared_ptr<char>>& queue, const size_t blockSize, bool& isFileReaded, std::string path)
{
    std::fstream fs;
    try{
        fs.open (path, std::fstream::in | std::fstream::binary);
        if (!fs)
            throw 0;
    }
    catch (...) {
        std::cout << "cant open file " << path << std::endl;
        isFileReaded = true;
        return;
    }

    fs.seekg (0, fs.end);
    size_t length = fs.tellg();
    fs.seekg (0, fs.beg);
    std::cout << "Reading " << length << " characters... " << std::endl;

    size_t index = 0;

    while (!fs.eof())
    {
        std::shared_ptr<char> bufPtr(new char[blockSize]);
        memset(bufPtr.get(), 0, blockSize);
        fs.read(bufPtr.get(), blockSize);
        queue.push(bufPtr, index);
        index++;
    }
    isFileReaded = true;
    fs.close();
}

void writeFileTask(const std::string& path, SafeQueue<uint32_t>& writeQueue, bool& isCalcEnded)
{
    std::fstream fs;
    try {
        fs.open(path, std::fstream::out | std::fstream::binary);
        if (!fs)
            throw 0;
    }
    catch (...) {
        std::cout << "cant open file " << path << std::endl;
        return;
    }

    while(!isCalcEnded || writeQueue.size())
    {
        std::pair<size_t, uint32_t> hash = writeQueue.frontAndPop();
        if (hash.second)
        {
            std::cout << hash.second << " crc of chunk " << hash.first << std::endl;
            fs.seekg(hash.first * sizeof(uint32_t));
            fs.write(reinterpret_cast<char*>(&hash.second), sizeof(uint32_t));
        }
    }
    fs.close();
}

int main() {
    std::cout << "Enter path for input file: " << std::endl;
    std::string inputFile;
    std::cin >> inputFile;

    std::cout << "Enter path for output file: " << std::endl;
    std::string outputFile;
    std::cin >> outputFile;

    std::cout << "Enter block size: " << std::endl;
    size_t blockSize = 1024*1024;
    std::cin >> blockSize;

    std::cout << "Enter thread count, min count 3: " << std::endl;
    std::cout << "1 thread for read, 1 for write and 1 calc hash. " << std::endl;
    size_t threadCount;
    std::cin >> threadCount;
    if (threadCount < 3)
        threadCount = 3;
    auto start = std::chrono::high_resolution_clock::now();

    SafeQueue<std::shared_ptr<char>> readQueue;
    SafeQueue<uint32_t> writeQueue;
    bool isFileReaded = false;
    bool isCalcEnded = false;
    std::future<void> readRes;
    std::future<void> writeRes;

    try {
        readRes = std::async(readFileTask, std::ref(readQueue), blockSize, std::ref(isFileReaded), inputFile);
    }
    catch (...) {
        std::cout << "Error with read task occurred" << std::endl;
        return 0;
    }
    try {
        writeRes = std::async(writeFileTask, outputFile, std::ref(writeQueue), std::ref(isCalcEnded));
    }
    catch (...) {
        std::cout << "Error with write task occurred" << std::endl;
        return 0;
    }

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < threadCount - 2; i++) {
        try {
            futures.push_back(
                    std::async(calcCrcTask, std::ref(readQueue), blockSize, std::ref(isFileReaded), std::ref(writeQueue), i));
        }
        catch (...) {
            std::cout << "Error with calc task occurred" << std::endl;
            return 0;
        }
    }

    for (size_t i = 0; i < threadCount - 2; i++)
        futures[i].wait();
    isCalcEnded = true;

    writeRes.wait();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time from start: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds" << std::endl;
    return 0;
}
