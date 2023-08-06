#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::priority_queue<std::function<void()>, std::vector<std::function<void()>>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.top());
                        this->tasks.pop();
                    }

                    task();
                }
            });
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(f);
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
            worker.join();
    }
};

bool executeAndSave(const std::string& command, const std::string& filename) {
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;
    std::ofstream file(filename);
    if (!file.is_open()) {
        pclose(pipe);
        return false;
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        file << buffer;
    pclose(pipe);
    file.close();
    return true;
}

int main() {
    ThreadPool pool(3); // 3 worker threads

    pool.enqueue([]{ executeAndSave("netstat -a", "netstat_output.txt"); });
    pool.enqueue([]{ executeAndSave("ifconfig -a", "ifconfig_output.txt"); });
    pool.enqueue([]{ executeAndSave("ps -aux", "ps_output.txt"); });
    
    // The thread pool will automatically join the threads in its destructor
}
