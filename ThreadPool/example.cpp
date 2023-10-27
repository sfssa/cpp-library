#include <iostream>
#include <chrono>
#include "ThreadPool.h"

int main() {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; ++i) {
        results.emplace_back(pool.enqueue([i] {
            try {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i * i;
            } catch (const std::exception& e) {
                std::cerr << "Exception in task: " << e.what() << std::endl;
                return -1;  // or handle the exception in an appropriate way
            }
        }));
    }

    for (auto&& result : results) {
        int value = result.get();
        std::cout << "Result: " << value << std::endl;
    }

    return 0;
}
