#pragma once
#include <windows.h>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>
namespace ThreadWork
{
    extern std::vector<int> DataBuffer;
    extern std::vector<bool> ActiveWorkers;
    extern HANDLE* PauseSignal;
    extern HANDLE* ResumeSignal;
    extern HANDLE* TerminateSignal;
    extern std::mutex DataMutex, IOMutex;
    extern LONG ActiveCount;
    extern int BufferSize;
}
void WorkerRoutine(int worker_id);