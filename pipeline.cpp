#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>
#include <sstream>
#include <thread>

#include "pool.hpp"
#include "blockingmutiqueue.hpp"

using MemBuf = std::span<std::uint8_t>;

int main() {
    constexpr size_t numBuffers = 5;
    constexpr size_t bufferSize = 128;
    constexpr size_t numQueues = 3;
    constexpr size_t numChunks = 15;

    // Create an array of buffers
    std::uint8_t buffers[numBuffers][bufferSize] = {};

    Pool<MemBuf> pool;
    BlockingMultiQueue<MemBuf> multiQueue(numQueues);

    // Populate pool
    for (int i = 0; i < numBuffers; ++i) {
        pool.push(MemBuf(buffers[i], bufferSize));
    }

    // Producer
    std::thread producer([&]() {
        for (int i = 0; i < numChunks; ++i) {
            MemBuf membuf = pool.pop();
            std::memset(membuf.data(), 0, membuf.size_bytes());
            std::string counterString = "Chunk #" + std::to_string(i + 1);
            std::strncpy((char*)membuf.data(), counterString.c_str(), bufferSize - 1);
            multiQueue.push(0, membuf);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // Consumers
    for (int i = 0; i < numChunks * numQueues; ++i) {
        auto [queueIndex, membuf] = multiQueue.popFromAny();
        std::cout << "Popped from queue " << queueIndex << ": " << reinterpret_cast<const char*>(membuf.data()) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(35));
        if (queueIndex < numQueues - 1)
            multiQueue.push(queueIndex + 1, membuf); // Pushing to next queue
        else
            pool.push(membuf);
    }

    producer.join();

    return 0;
}
