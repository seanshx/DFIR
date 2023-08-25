#include <windows.h>
#include <iostream>

const int THREAD_COUNT = 6;
const int MAX_CONCURRENT_THREADS = 3;

// The semaphore handle
HANDLE hSemaphore;

// Six separate functions for the threads
void Function1() {
    std::cout << "Function 1 running." << std::endl;
    Sleep(1000); // Simulate some work by sleeping
}

void Function2() {
    std::cout << "Function 2 running." << std::endl;
    Sleep(1000);
}

void Function3() {
    std::cout << "Function 3 running." << std::endl;
    Sleep(1000);
}

void Function4() {
    std::cout << "Function 4 running." << std::endl;
    Sleep(1000);
}

void Function5() {
    std::cout << "Function 5 running." << std::endl;
    Sleep(1000);
}

void Function6() {
    std::cout << "Function 6 running." << std::endl;
    Sleep(1000);
}

// The general thread function to be queued
DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    // Wait on the semaphore
    WaitForSingleObject(hSemaphore, INFINITE);

    // Call the appropriate function based on the passed parameter
    void (*func)() = (void(*)())lpParam;
    func();

    // Release the semaphore
    ReleaseSemaphore(hSemaphore, 1, NULL);

    return 0;
}

int main() {
    hSemaphore = CreateSemaphore(NULL, MAX_CONCURRENT_THREADS, MAX_CONCURRENT_THREADS, NULL);
    if (hSemaphore == NULL) {
        std::cerr << "Failed to create semaphore. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    // An array of function pointers to the six functions
    void (*functions[THREAD_COUNT])() = {Function1, Function2, Function3, Function4, Function5, Function6};

    // Queue threads using QueueUserWorkItem
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (!QueueUserWorkItem(ThreadFunction, functions[i], 0)) {
            std::cerr << "Failed to queue thread. Error code: " << GetLastError() << std::endl;
            CloseHandle(hSemaphore);
            return 1;
        }
    }

    // Waiting for all threads to complete their execution
    for (int i = 0; i < MAX_CONCURRENT_THREADS; i++) {
        WaitForSingleObject(hSemaphore, INFINITE);
    }
    
    // At this point, all threads are finished, so it's safe to close the semaphore handle
    CloseHandle(hSemaphore);

    std::cout << "All threads completed." << std::endl;

    return 0;
}
