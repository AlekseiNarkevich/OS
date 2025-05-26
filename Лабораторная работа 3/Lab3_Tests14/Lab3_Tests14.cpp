#include "gtest/gtest.h"
#include <windows.h>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

namespace ThreadWork {
    inline std::vector<int> DataBuffer;
    inline std::vector<bool> ActiveWorkers;
    inline HANDLE* PauseSignal;
    inline HANDLE* ResumeSignal;
    inline HANDLE* TerminateSignal;
    inline std::mutex DataMutex, IOMutex;
    inline LONG ActiveCount;
    inline int BufferSize;

    inline void WorkerRoutine(int worker_id) {
        srand(worker_id + 1);
        int modifications = 0;

        while (true) {
            int target_index = rand() % BufferSize;

            {
                std::lock_guard<std::mutex> lock(DataMutex);
                if (DataBuffer[target_index] == 0) {
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

            HANDLE handles[2] = { TerminateSignal[worker_id], ResumeSignal[worker_id] };
            DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

            if (result == WAIT_OBJECT_0) {
                {
                    std::lock_guard<std::mutex> lock(DataMutex);
                    for (auto& val : DataBuffer) {
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
}

class Lab3Test : public ::testing::Test {
protected:
    void SetUp() override {
        ThreadWork::BufferSize = 10;
        ThreadWork::DataBuffer = std::vector<int>(ThreadWork::BufferSize, 0);
        ThreadWork::ActiveCount = 3;
        ThreadWork::ActiveWorkers = std::vector<bool>(3, true);

        ThreadWork::PauseSignal = new HANDLE[3];
        ThreadWork::ResumeSignal = new HANDLE[3];
        ThreadWork::TerminateSignal = new HANDLE[3];

        for (int i = 0; i < 3; ++i) {
            ThreadWork::PauseSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
            ThreadWork::ResumeSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
            ThreadWork::TerminateSignal[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        }
    }

    void TearDown() override {
        for (int i = 0; i < 3; ++i) {
            CloseHandle(ThreadWork::PauseSignal[i]);
            CloseHandle(ThreadWork::ResumeSignal[i]);
            CloseHandle(ThreadWork::TerminateSignal[i]);
        }
        delete[] ThreadWork::PauseSignal;
        delete[] ThreadWork::ResumeSignal;
        delete[] ThreadWork::TerminateSignal;
    }
};

TEST_F(Lab3Test, WorkerMarksCellsCorrectly) {
    std::thread worker(ThreadWork::WorkerRoutine, 0);
    Sleep(50);

    bool marked = false;
    {
        std::lock_guard<std::mutex> lock(ThreadWork::DataMutex);
        for (int val : ThreadWork::DataBuffer) {
            if (val == 10) {
                marked = true;
                break;
            }
        }
    }

    ASSERT_TRUE(marked) << "Worker should mark cells with its ID";

    SetEvent(ThreadWork::TerminateSignal[0]);
    worker.join();
}

TEST_F(Lab3Test, WorkerTerminationClearsCells) {
    {
        std::lock_guard<std::mutex> lock(ThreadWork::DataMutex);
        ThreadWork::DataBuffer[0] = 10;
        ThreadWork::DataBuffer[1] = 11;
        ThreadWork::DataBuffer[2] = 10;
    }

    std::thread worker(ThreadWork::WorkerRoutine, 0);
    SetEvent(ThreadWork::TerminateSignal[0]);
    worker.join();

    {
        std::lock_guard<std::mutex> lock(ThreadWork::DataMutex);
        EXPECT_EQ(ThreadWork::DataBuffer[0], 0) << "Worker 0 should clear its marks";
        EXPECT_EQ(ThreadWork::DataBuffer[2], 0) << "Worker 0 should clear its marks";
        EXPECT_EQ(ThreadWork::DataBuffer[1], 11) << "Other workers' marks should remain";
    }
}

TEST_F(Lab3Test, MutexProtection) {
    std::unique_lock<std::mutex> lock(ThreadWork::DataMutex);
    ThreadWork::DataBuffer[0] = 10;

    bool accessGranted = false;
    std::thread testThread([&accessGranted]() {
        std::unique_lock<std::mutex> testLock(ThreadWork::DataMutex, std::try_to_lock);
        accessGranted = testLock.owns_lock();
        });

    testThread.join();
    EXPECT_FALSE(accessGranted) << "Mutex should block other threads when locked";
    lock.unlock();
}

TEST_F(Lab3Test, EventSignalingWorks) {
    std::thread worker(ThreadWork::WorkerRoutine, 0);
    Sleep(50);

    {
        std::lock_guard<std::mutex> lock(ThreadWork::DataMutex);
        ThreadWork::DataBuffer[0] = 99;
    }

    DWORD waitResult = WaitForSingleObject(ThreadWork::PauseSignal[0], 1000);
    EXPECT_EQ(waitResult, WAIT_OBJECT_0) << "Worker should signal when blocked";

    SetEvent(ThreadWork::ResumeSignal[0]);
    SetEvent(ThreadWork::TerminateSignal[0]);
    worker.join();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}