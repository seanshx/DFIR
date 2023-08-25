#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

const int MAX_CONCURRENT_THREADS = 3;

class FunctionTask {
public:
    virtual void execute() = 0;
};

class Function1 : public FunctionTask {
    void execute() override {
        std::cout << "Function 1 running." << std::endl;
        Sleep(1000);
    }
};

class Function2 : public FunctionTask {
    void execute() override {
        std::cout << "Function 2 running." << std::endl;
        Sleep(1000);
    }
};

// ... Similarly for Function3 to Function6 ...

class ThreadPool {
private:
    HANDLE hSemaphore;
    std::vector<HANDLE> hEvents;
    std::vector<FunctionTask*> enabledTasks;

    static DWORD WINAPI ThreadFunction(LPVOID lpParam) {
        FunctionTask* task = (FunctionTask*)lpParam;
        WaitForSingleObject(task->hSemaphore, INFINITE);
        task->execute();
        ReleaseSemaphore(task->hSemaphore, 1, NULL);
        SetEvent(task->hEvent);
        return 0;
    }

public:
    ThreadPool() {
        hSemaphore = CreateSemaphore(NULL, MAX_CONCURRENT_THREADS, MAX_CONCURRENT_THREADS, NULL);
    }

    ~ThreadPool() {
        CloseHandle(hSemaphore);
        for (HANDLE event : hEvents) {
            CloseHandle(event);
        }
    }

    void addTask(FunctionTask* task) {
        enabledTasks.push_back(task);
        hEvents.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));
    }

    void start() {
        for (size_t i = 0; i < enabledTasks.size(); i++) {
            QueueUserWorkItem(ThreadFunction, enabledTasks[i], 0);
        }

        WaitForMultipleObjects((DWORD)hEvents.size(), &hEvents[0], TRUE, INFINITE);
    }
};

json ReadConfigFile(const std::string& filepath) {
    std::ifstream configFile(filepath);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open the configuration file." << std::endl;
        exit(1);
    }

    json config;
    configFile >> config;
    return config;
}

int main() {
    json config = ReadConfigFile("config.json");
    std::cout << "Configuration settings:\n" << config.dump(4) << std::endl;

    ThreadPool pool;

    if (config["Function1"]["isEnabled"]) pool.addTask(new Function1());
    if (config["Function2"]["isEnabled"]) pool.addTask(new Function2());
    // ... Similarly for Function3 to Function6 ...

    pool.start();

    std::cout << "All enabled tasks completed." << std::endl;

    return 0;
}
