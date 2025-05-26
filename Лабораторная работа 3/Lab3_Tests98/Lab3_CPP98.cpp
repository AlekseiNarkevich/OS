#include "Lab3_CPP98.h"

namespace ThreadWork {
    std::vector<int> DataBuffer;
    std::vector<bool> ActiveWorkers;
    HANDLE* PauseSignal;
    HANDLE* ResumeSignal;
    HANDLE* TerminateSignal;
    CRITICAL_SECTION DataLock, IOLock;
    LONG ActiveCount;
    int BufferSize;
}

using namespace ThreadWork;

DWORD WINAPI WorkerRoutine(LPVOID Context) {
    const int worker_id = reinterpret_cast<int>(Context);
    srand(worker_id + 1);
    int modifications = 0;

    while (true) {
        const int target_index = rand() % BufferSize;

        EnterCriticalSection(&DataLock);
        if (DataBuffer[target_index] == 0) {
            Sleep(7);
            DataBuffer[target_index] = worker_id + 10;
            modifications++;
            LeaveCriticalSection(&DataLock);
            Sleep(3);
        }
        else {
            LeaveCriticalSection(&DataLock);

            EnterCriticalSection(&IOLock);
            std::cout << "[Worker #" << worker_id
                << "] Marked cells: " << modifications
                << " | Conflict at: " << target_index << '\n';
            SetEvent(PauseSignal[worker_id]);
            LeaveCriticalSection(&IOLock);

            HANDLE sync_objects[] = { TerminateSignal[worker_id], ResumeSignal[worker_id] };
            switch (WaitForMultipleObjects(2, sync_objects, FALSE, INFINITE)) {
            case WAIT_OBJECT_0: // Termination
                EnterCriticalSection(&DataLock);
                for (size_t i = 0; i < DataBuffer.size(); ++i) {
                    if (DataBuffer[i] == worker_id + 10) DataBuffer[i] = 0;
                }
                LeaveCriticalSection(&DataLock);

                EnterCriticalSection(&IOLock);
                std::cout << "## Worker #" << worker_id << " terminated ##\n";
                LeaveCriticalSection(&IOLock);

                ActiveWorkers[worker_id] = false;
                InterlockedDecrement(&ActiveCount);
                return 0;

            case WAIT_OBJECT_0 + 1: // Resume
                ResetEvent(PauseSignal[worker_id]);
                break;
            }
        }
    }
}

int main() {
    std::cout << "Specify buffer capacity: ";
    std::cin >> BufferSize;
    DataBuffer = std::vector<int>(BufferSize, 0); // Инициализация нулями

    int workers_total;
    std::cout << "Set number of workers: ";
    std::cin >> workers_total;
    ActiveCount = workers_total;

    InitializeCriticalSection(&DataLock);
    InitializeCriticalSection(&IOLock);

    PauseSignal = new HANDLE[workers_total];
    ResumeSignal = new HANDLE[workers_total];
    TerminateSignal = new HANDLE[workers_total];
    HANDLE* workers = new HANDLE[workers_total];
    ActiveWorkers = std::vector<bool>(workers_total, true);

    for (int i = 0; i < workers_total; ++i) {
        PauseSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        ResumeSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        TerminateSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        workers[i] = CreateThread(NULL, 0, WorkerRoutine, reinterpret_cast<LPVOID>(i), 0, NULL);
    }

    while (ActiveCount > 0) {
        std::vector<HANDLE> active_pauses;
        for (int i = 0; i < workers_total; ++i) {
            if (ActiveWorkers[i]) active_pauses.push_back(PauseSignal[i]);
        }
        WaitForMultipleObjects(active_pauses.size(), &active_pauses[0], TRUE, INFINITE);

        EnterCriticalSection(&IOLock);
        std::cout << "\nCurrent buffer state [ ";
        for (int val : DataBuffer) std::cout << val << ' ';
        std::cout << "]\n";
        LeaveCriticalSection(&IOLock);

        int target_id;
        bool valid = false;
        do {
            std::cout << "Select worker to terminate (0-" << workers_total - 1 << "): ";
            if (!(std::cin >> target_id)) {
                std::cin.clear();
                std::cin.ignore(INT_MAX, '\n');
            }
            valid = (target_id >= 0) && (target_id < workers_total) && ActiveWorkers[target_id];
            if (!valid) std::cout << "Invalid selection!\n";
        } while (!valid);

        SetEvent(TerminateSignal[target_id]);
        WaitForSingleObject(workers[target_id], INFINITE);
        CloseHandle(workers[target_id]);

        EnterCriticalSection(&IOLock);
        std::cout << "Buffer after cleanup: [ ";
        for (int val : DataBuffer) std::cout << val << ' ';
        std::cout << "]\n\n";
        LeaveCriticalSection(&IOLock);

        for (int i = 0; i < workers_total; ++i) {
            if (ActiveWorkers[i]) {
                ResetEvent(PauseSignal[i]);
                SetEvent(ResumeSignal[i]);
            }
        }
    }

    DeleteCriticalSection(&DataLock);
    DeleteCriticalSection(&IOLock);
    for (int i = 0; i < workers_total; ++i) {
        CloseHandle(PauseSignal[i]);
        CloseHandle(ResumeSignal[i]);
        CloseHandle(TerminateSignal[i]);
    }
    delete[] workers;
    delete[] PauseSignal;
    delete[] ResumeSignal;
    delete[] TerminateSignal;

    std::cout << "\n====== All workers finished ======\n";
    return 0;
}