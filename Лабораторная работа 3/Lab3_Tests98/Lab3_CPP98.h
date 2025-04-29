#pragma once
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <vector>
#include <iostream>

namespace ThreadWork {
    extern std::vector<int> DataBuffer;
    extern std::vector<bool> ActiveWorkers;
    extern HANDLE* PauseSignal;
    extern HANDLE* ResumeSignal;
    extern HANDLE* TerminateSignal;
    extern CRITICAL_SECTION DataLock, IOLock;
    extern LONG ActiveCount;
    extern int BufferSize;
}

DWORD WINAPI WorkerRoutine(LPVOID Context);