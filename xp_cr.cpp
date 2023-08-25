#include <windows.h>
#include <iostream>
#include <vector>

const int THREAD_COUNT = 6;
const int MAX_CONCURRENT_THREADS = 3;

// Semaphore pointer
HANDLE* hSemaphore = nullptr;

// Array of event handles to determine thread completion
HANDLE hEvents[THREAD_COUNT];

// Six separate functions
void Function1() {
    std::cout << "Function 1 running." << std::endl;
    Sleep(1000);
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

// The general thread function
DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    // Wait on the semaphore
    WaitForSingleObject(*hSemaphore, INFINITE);

    // Call the appropriate function
    void (*func)() = (void(*)())lpParam;
    func();

    // Release the semaphore
    ReleaseSemaphore(*hSemaphore, 1, NULL);

    // Signal that this thread is done
    int index = func == Function1 ? 0 : func == Function2 ? 1 : func == Function3 ? 2 : func == Function4 ? 3 : func == Function5 ? 4 : 5;
    SetEvent(hEvents[index]);

    return 0;
}

int main() {
    hSemaphore = new HANDLE;
    *hSemaphore = CreateSemaphore(NULL, MAX_CONCURRENT_THREADS, MAX_CONCURRENT_THREADS, NULL);
    if (*hSemaphore == NULL) {
        std::cerr << "Failed to create semaphore. Error code: " << GetLastError() << std::endl;
        delete hSemaphore;
        return 1;
    }

    // Initialize events
    for (int i = 0; i < THREAD_COUNT; i++) {
        hEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (hEvents[i] == NULL) {
            std::cerr << "Failed to create event. Error code: " << GetLastError() << std::endl;
            CloseHandle(*hSemaphore);
            delete hSemaphore;
            for (int j = 0; j < i; j++) {
                CloseHandle(hEvents[j]);
            }
            return 1;
        }
    }

    // A vector of function pointers
    std::vector<void(*)()> functions = {Function1, Function2, Function3, Function4, Function5, Function6};

    // Queue threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (!QueueUserWorkItem(ThreadFunction, functions[i], 0)) {
            std::cerr << "Failed to queue thread. Error code: " << GetLastError() << std::endl;
            CloseHandle(*hSemaphore);
            delete hSemaphore;
            for (int j = 0; j < THREAD_COUNT; j++) {
                CloseHandle(hEvents[j]);
            }
            return 1;
        }
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(THREAD_COUNT, hEvents, TRUE, INFINITE);

    // Cleanup
    CloseHandle(*hSemaphore);
    delete hSemaphore;
    for (int i = 0; i < THREAD_COUNT; i++) {
        CloseHandle(hEvents[i]);
    }

    std::cout << "All threads completed." << std::endl;

    return 0;
}
