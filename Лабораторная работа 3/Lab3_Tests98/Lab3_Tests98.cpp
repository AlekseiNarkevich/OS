#include "gtest/gtest.h"
#include "Lab3_CPP98.h"
namespace ThreadWork
{
    std::vector<int> DataBuffer;
    std::vector<bool> ActiveWorkers;
    HANDLE* PauseSignal;
    HANDLE* ResumeSignal;
    HANDLE* TerminateSignal;
    CRITICAL_SECTION DataLock, IOLock;
    LONG ActiveCount;
    int BufferSize;
}
DWORD WINAPI WorkerRoutine(LPVOID Context) 
{
    const int worker_id = reinterpret_cast<int>(Context);
    srand(worker_id + 1);
    int modifications = 0;
    while (true) 
    {
        const int target_index = rand() % ThreadWork::BufferSize;
        EnterCriticalSection(&ThreadWork::DataLock);
        if (ThreadWork::DataBuffer[target_index] == 0)
        {
            Sleep(7);
            ThreadWork::DataBuffer[target_index] = worker_id + 10;
            modifications++;
            LeaveCriticalSection(&ThreadWork::DataLock);
            Sleep(3);
        }
        else
        {
            LeaveCriticalSection(&ThreadWork::DataLock);
            EnterCriticalSection(&ThreadWork::IOLock);
            std::cout << "[Worker #" << worker_id
                << "] Marked cells: " << modifications
                << " | Conflict at: " << target_index << '\n';
            SetEvent(ThreadWork::PauseSignal[worker_id]);
            LeaveCriticalSection(&ThreadWork::IOLock);
            HANDLE sync_objects[] = 
            {
                ThreadWork::TerminateSignal[worker_id],
                ThreadWork::ResumeSignal[worker_id]
            };
            switch (WaitForMultipleObjects(2, sync_objects, FALSE, INFINITE)) 
            {
            case WAIT_OBJECT_0:
                EnterCriticalSection(&ThreadWork::DataLock);
                for (size_t i = 0; i < ThreadWork::DataBuffer.size(); ++i)
                {
                    if (ThreadWork::DataBuffer[i] == worker_id + 10)
                    {
                        ThreadWork::DataBuffer[i] = 0;
                    }
                }
                LeaveCriticalSection(&ThreadWork::DataLock);
                EnterCriticalSection(&ThreadWork::IOLock);
                std::cout << "## Worker #" << worker_id << " terminated ##\n";
                LeaveCriticalSection(&ThreadWork::IOLock);
                ThreadWork::ActiveWorkers[worker_id] = false;
                InterlockedDecrement(&ThreadWork::ActiveCount);
                return 0;
            case WAIT_OBJECT_0 + 1:
                ResetEvent(ThreadWork::PauseSignal[worker_id]);
                break;
            }
        }
    }
}
class Lab3Test : public ::testing::Test
{
protected:
    void SetUp() override 
    {
        ThreadWork::BufferSize = 10;
        ThreadWork::DataBuffer = std::vector<int>(ThreadWork::BufferSize, 0);
        ThreadWork::ActiveCount = 3;
        ThreadWork::ActiveWorkers = std::vector<bool>(3, true);
        InitializeCriticalSection(&ThreadWork::DataLock);
        InitializeCriticalSection(&ThreadWork::IOLock);
        ThreadWork::PauseSignal = new HANDLE[3];
        ThreadWork::ResumeSignal = new HANDLE[3];
        ThreadWork::TerminateSignal = new HANDLE[3];
        for (int i = 0; i < 3; ++i) 
        {
            ThreadWork::PauseSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
            ThreadWork::ResumeSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
            ThreadWork::TerminateSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        }
    }
    void TearDown() override
    {
        DeleteCriticalSection(&ThreadWork::DataLock);
        DeleteCriticalSection(&ThreadWork::IOLock);
        for (int i = 0; i < 3; ++i)
        {
            CloseHandle(ThreadWork::PauseSignal[i]);
            CloseHandle(ThreadWork::ResumeSignal[i]);
            CloseHandle(ThreadWork::TerminateSignal[i]);
        }
        delete[] ThreadWork::PauseSignal;
        delete[] ThreadWork::ResumeSignal;
        delete[] ThreadWork::TerminateSignal;
    }
};
TEST_F(Lab3Test, WorkerMarksCellsCorrectly)
{
    HANDLE hThread = CreateThread(NULL, 0, WorkerRoutine, (LPVOID)0, 0, NULL);
    Sleep(50);
    bool marked = false;
    EnterCriticalSection(&ThreadWork::DataLock);
    for (int val : ThreadWork::DataBuffer) 
    {
        if (val == 10)
        {
            marked = true;
            break;
        }
    }
    LeaveCriticalSection(&ThreadWork::DataLock);
    ASSERT_TRUE(marked) << "Worker should mark cells with its ID";
    SetEvent(ThreadWork::TerminateSignal[0]);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}
TEST_F(Lab3Test, WorkerTerminationClearsCells)
{
    EnterCriticalSection(&ThreadWork::DataLock);
    ThreadWork::DataBuffer[0] = 10;
    ThreadWork::DataBuffer[1] = 11;
    ThreadWork::DataBuffer[2] = 10;
    LeaveCriticalSection(&ThreadWork::DataLock);
    HANDLE hThread = CreateThread(NULL, 0, WorkerRoutine, (LPVOID)0, 0, NULL);
    SetEvent(ThreadWork::TerminateSignal[0]);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    EnterCriticalSection(&ThreadWork::DataLock);
    EXPECT_EQ(ThreadWork::DataBuffer[0], 0) << "Worker 0 should clear its marks";
    EXPECT_EQ(ThreadWork::DataBuffer[2], 0) << "Worker 0 should clear its marks";
    EXPECT_EQ(ThreadWork::DataBuffer[1], 11) << "Other workers' marks should remain";
    LeaveCriticalSection(&ThreadWork::DataLock);
}
TEST_F(Lab3Test, CriticalSectionProtection) 
{
    EnterCriticalSection(&ThreadWork::DataLock);
    ThreadWork::DataBuffer[0] = 10;
    DWORD result = WaitForSingleObject(
        CreateThread(NULL, 0, [](LPVOID) -> DWORD {
            EnterCriticalSection(&ThreadWork::DataLock);
            bool canAccess = (ThreadWork::DataBuffer[0] == 10);
            LeaveCriticalSection(&ThreadWork::DataLock);
            return canAccess ? 0 : 1;
            }, NULL, 0, NULL),
        1000
    );
    EXPECT_EQ(result, WAIT_TIMEOUT) << "Other thread should be blocked by critical section";
    ThreadWork::DataBuffer[0] = 0;
    LeaveCriticalSection(&ThreadWork::DataLock);
}
TEST_F(Lab3Test, EventSignalingWorks)
{
    HANDLE hThread = CreateThread(NULL, 0, WorkerRoutine, (LPVOID)0, 0, NULL);
    Sleep(50);
    EnterCriticalSection(&ThreadWork::DataLock);
    ThreadWork::DataBuffer[0] = 99;
    LeaveCriticalSection(&ThreadWork::DataLock);
    DWORD waitResult = WaitForSingleObject(ThreadWork::PauseSignal[0], 1000);
    EXPECT_EQ(waitResult, WAIT_OBJECT_0) << "Worker should signal when blocked";
    SetEvent(ThreadWork::ResumeSignal[0]);
    SetEvent(ThreadWork::TerminateSignal[0]);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}
int main(int argc, char** argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}