#include "Lab3_CPP14.h"
namespace ThreadWork 
{
    std::vector<int> DataBuffer;
    std::vector<bool> ActiveWorkers;
    HANDLE* PauseSignal;
    HANDLE* ResumeSignal;
    HANDLE* TerminateSignal;
    std::mutex DataMutex, IOMutex;
    LONG ActiveCount;
    int BufferSize;
}
using namespace ThreadWork;
void WorkerRoutine(int worker_id)
{
    srand(worker_id + 1);
    int modifications = 0;
    while (true)
    {
        int target_index = rand() % BufferSize;
        {
            std::lock_guard<std::mutex> lock(DataMutex);
            if (DataBuffer[target_index] == 0) 
            {
                Sleep(7);
                DataBuffer[target_index] = worker_id + 10;
                modifications++;
                continue;
            }
        }
        {
            std::lock_guard<std::mutex> lock(IOMutex);
            std::cout << "[Worker #" << worker_id
                << "] Marked cells: " << modifications
                << " | Conflict at: " << target_index << '\n';
            SetEvent(PauseSignal[worker_id]);
        }
        HANDLE handles[2] = {TerminateSignal[worker_id], ResumeSignal[worker_id]};
        DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (result == WAIT_OBJECT_0) 
        {
            {
                std::lock_guard<std::mutex> lock(DataMutex);
                for (auto& val : DataBuffer)
                {
                    if (val == worker_id + 10) val = 0;
                }
            }
            {
                std::lock_guard<std::mutex> lock(IOMutex);
                std::cout << "## Worker #" << worker_id << " terminated ##\n";
            }
            ActiveWorkers[worker_id] = false;
            InterlockedDecrement(&ActiveCount);
            return;
        }
        ResetEvent(PauseSignal[worker_id]);
    }
}
int main() 
{
    std::cout << "Specify buffer capacity: ";
    std::cin >> BufferSize;
    DataBuffer = std::vector<int>(BufferSize, 0);
    int workers_total;
    std::cout << "Set number of workers: ";
    std::cin >> workers_total;
    ActiveCount = workers_total;
    PauseSignal = new HANDLE[workers_total];
    ResumeSignal = new HANDLE[workers_total];
    TerminateSignal = new HANDLE[workers_total];
    ActiveWorkers = std::vector<bool>(workers_total, true);
    std::vector<std::thread> workers;
    for (int i = 0; i < workers_total; ++i) 
    {
        PauseSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        ResumeSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        TerminateSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        workers.emplace_back(WorkerRoutine, i);
    }
    while (ActiveCount > 0)
    {
        std::vector<HANDLE> active_pauses;
        for (int i = 0; i < workers_total; ++i) 
        {
            if (ActiveWorkers[i]) active_pauses.push_back(PauseSignal[i]);
        }
        WaitForMultipleObjects(active_pauses.size(), &active_pauses[0], TRUE, INFINITE);
        {
            std::lock_guard<std::mutex> lock(IOMutex);
            std::cout << "\nCurrent buffer state [ ";
            for (int val : DataBuffer) std::cout << val << ' ';
            std::cout << "]\n";
        }
        int target_id;
        bool valid = false;
        do 
        {
            std::cout << "Select worker to terminate (0-" << workers_total - 1 << "): ";
            if (!(std::cin >> target_id))
            {
                std::cin.clear();
                std::cin.ignore(INT_MAX, '\n');
            }
            valid = (target_id >= 0) && (target_id < workers_total) && ActiveWorkers[target_id];
            if (!valid) std::cout << "Invalid selection!\n";
        } while (!valid);
        SetEvent(TerminateSignal[target_id]);
        workers[target_id].join();
        {
            std::lock_guard<std::mutex> lock(IOMutex);
            std::cout << "Buffer after cleanup: [ ";
            for (int val : DataBuffer) std::cout << val << ' ';
            std::cout << "]\n\n";
        }
        for (int i = 0; i < workers_total; ++i)
        {
            if (ActiveWorkers[i]) 
            {
                ResetEvent(PauseSignal[i]);
                SetEvent(ResumeSignal[i]);
            }
        }
    }
    for (int i = 0; i < workers_total; ++i) 
    {
        CloseHandle(PauseSignal[i]);
        CloseHandle(ResumeSignal[i]);
        CloseHandle(TerminateSignal[i]);
    }
    delete[] PauseSignal;
    delete[] ResumeSignal;
    delete[] TerminateSignal;
    std::cout << "\n====== All workers finished ======\n";
    return 0;
}