#include <condition_variable>
#include <list>
#include <mutex>
#include <utility> // For std::pair
#include <vector>

template<typename T>
class BlockingMultiQueue {
private:
    std::vector<std::list<T>> queues;
    std::mutex mtx;
    std::condition_variable cv;

public:
    BlockingMultiQueue(size_t numQueues) : queues(numQueues) {}

    void push(size_t index, const T& value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queues[index].push_back(value);
        }
        cv.notify_one();
    }

    std::pair<size_t, T> popFromAny() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return hasDataInAnyQueue(); });

        for (size_t i = 0; i < queues.size(); ++i) {
            if (!queues[i].empty()) {
                T value = queues[i].front();
                queues[i].pop_front();
                return {i, value};
            }
        }

        throw std::logic_error("Unexpected empty queue");
    }

private:
    bool hasDataInAnyQueue() const {
        for (const auto& queue : queues) {
            if (!queue.empty()) {
                return true;
            }
        }
        return false;
    }
};
