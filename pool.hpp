#include <condition_variable>
#include <list>
#include <mutex>

template<typename T>
class Pool {
private:
    std::list<T> pool;
    std::mutex mtx;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        pool.push_back(value);
    }

    T pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!pool.empty()) {
            T value = pool.front();
            pool.pop_front();
            return value;
        }

        throw std::logic_error("Unexpected empty list");
    }
};
