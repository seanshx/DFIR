#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

const int THREAD_COUNT = 6;
const int MAX_CONCURRENT_THREADS = 3;

std::mutex mtx;                     // Mutex for controlling access to std::cout
std::mutex semaphore_mtx;          // Mutex for the semaphore logic
std::condition_variable cv;        // Condition variable for the semaphore logic
int available_threads = MAX_CONCURRENT_THREADS;

void Function1() {
    std::cout << "Function 1 running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Function2() {
    std::cout << "Function 2 running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Function3() {
    std::cout << "Function 3 running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Function4() {
    std::cout << "Function 4 running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Function5() {
    std::cout << "Function 5 running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Function6() {
    std::cout << "Function 6 running." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void ThreadFunction(void (*func)()) {
    {
        std::unique_lock<std::mutex> lock(semaphore_mtx);
        cv.wait(lock, [] { return available_threads > 0; });
        --available_threads;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        func();
    }

    {
        std::lock_guard<std::mutex> lock(semaphore_mtx);
        ++available_threads;
        cv.notify_one();
    }
}

int main() {
    // A vector of function pointers
    std::vector<void(*)()> functions = {Function1, Function2, Function3, Function4, Function5, Function6};

    std::vector<std::thread> threads;

    for (int i = 0; i < THREAD_COUNT; i++) {
        threads.emplace_back(ThreadFunction, functions[i]);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "All threads completed." << std::endl;

    return 0;
}
