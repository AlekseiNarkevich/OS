#include <gtest/gtest.h>
#include <windows.h>
#include <cassert>
#include <string>
#include "average.h"
#include "min_max.h"
#include "data.h"

class ThreadTestFixture : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        InitializeCriticalSection(&Data::cs);
    }

    static void TearDownTestCase() {
        DeleteCriticalSection(&Data::cs);
    }
};

template<typename Function>
void RunThreadTest(Data& data, Function threadFunc) {
    HANDLE hThread = CreateThread(NULL, 0, threadFunc, &data, 0, NULL);
    ASSERT_TRUE(hThread != NULL)
        << "Ошибка создания потока. Код: " << GetLastError();

    const DWORD waitResult = WaitForSingleObject(hThread, INFINITE);
    ASSERT_EQ(waitResult, WAIT_OBJECT_0)
        << "Ошибка ожидания потока. Код: " << GetLastError();

    CloseHandle(hThread);
}

// Тесты для calculateAverage
TEST_F(ThreadTestFixture, AveragePositiveNumbers) {
    Data data;
    double numbers[] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    data.numbers.assign(numbers, numbers + 5);

    RunThreadTest(data, calculateAverage);
    EXPECT_DOUBLE_EQ(3.0, data.average);
}

TEST_F(ThreadTestFixture, AverageNegativeNumbers) {
    Data data;
    double numbers[] = { -1.0, -2.0, -3.0, -4.0, -5.0 };
    data.numbers.assign(numbers, numbers + 5);

    RunThreadTest(data, calculateAverage);
    EXPECT_DOUBLE_EQ(-3.0, data.average);
}

TEST_F(ThreadTestFixture, AverageMixedNumbers) {
    Data data;
    double numbers[] = { -1.0, 2.0, -3.0, 4.0, -5.0 };
    data.numbers.assign(numbers, numbers + 5);

    RunThreadTest(data, calculateAverage);
    EXPECT_DOUBLE_EQ(-0.6, data.average);
}

TEST_F(ThreadTestFixture, AverageSingleElement) {
    Data data;
    data.numbers.push_back(42.0);

    RunThreadTest(data, calculateAverage);
    EXPECT_DOUBLE_EQ(42.0, data.average);
}

// Тесты для findMinMax
TEST_F(ThreadTestFixture, MinMaxPositiveNumbers) {
    Data data;
    double numbers[] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    data.numbers.assign(numbers, numbers + 5);

    RunThreadTest(data, findMinMax);
    EXPECT_DOUBLE_EQ(1.0, data.min);
    EXPECT_DOUBLE_EQ(5.0, data.max);
}

TEST_F(ThreadTestFixture, MinMaxNegativeNumbers) {
    Data data;
    double numbers[] = { -1.0, -2.0, -3.0, -4.0, -5.0 };
    data.numbers.assign(numbers, numbers + 5);

    RunThreadTest(data, findMinMax);
    EXPECT_DOUBLE_EQ(-5.0, data.min);
    EXPECT_DOUBLE_EQ(-1.0, data.max);
}

TEST_F(ThreadTestFixture, MinMaxMixedNumbers) {
    Data data;
    double numbers[] = { -1.0, 2.0, -3.0, 4.0, -5.0 };
    data.numbers.assign(numbers, numbers + 5);

    RunThreadTest(data, findMinMax);
    EXPECT_DOUBLE_EQ(-5.0, data.min);
    EXPECT_DOUBLE_EQ(4.0, data.max);
}

TEST_F(ThreadTestFixture, MinMaxSingleElement) {
    Data data;
    data.numbers.push_back(42.0);

    RunThreadTest(data, findMinMax);
    EXPECT_DOUBLE_EQ(42.0, data.min);
    EXPECT_DOUBLE_EQ(42.0, data.max);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    InitializeCriticalSection(&Data::cs);
    int result = RUN_ALL_TESTS();
    DeleteCriticalSection(&Data::cs);

    return result;
}