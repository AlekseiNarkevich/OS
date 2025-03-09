#include <gtest/gtest.h>
#include <windows.h>
#include "average.h"
#include "min_max.h"
#include "data.h"

// Тесты calculateAverage
TEST(AverageTest, CalculatesAverageForPositiveNumbers) {
    Data data;
    data.numbers = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(3.0, data.average);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

TEST(AverageTest, CalculatesAverageForNegativeNumbers) {
    Data data;
    data.numbers = { -1.0, -2.0, -3.0, -4.0, -5.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(-3.0, data.average);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

TEST(AverageTest, CalculatesAverageForMixedNumbers) {
    Data data;
    data.numbers = { -1.0, 2.0, -3.0, 4.0, -5.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(-0.6, data.average);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

TEST(AverageTest, HandlesSingleElement) {
    Data data;
    data.numbers = { 42.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(42.0, data.average);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

// Тесты findMinMax
TEST(MinMaxTest, FindsMinMaxForPositiveNumbers) {
    Data data;
    data.numbers = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(1.0, data.min);
    EXPECT_DOUBLE_EQ(5.0, data.max);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

TEST(MinMaxTest, FindsMinMaxForNegativeNumbers) {
    Data data;
    data.numbers = { -1.0, -2.0, -3.0, -4.0, -5.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(-5.0, data.min);
    EXPECT_DOUBLE_EQ(-1.0, data.max);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

TEST(MinMaxTest, FindsMinMaxForMixedNumbers) {
    Data data;
    data.numbers = { -1.0, 2.0, -3.0, 4.0, -5.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(-5.0, data.min);
    EXPECT_DOUBLE_EQ(4.0, data.max); 

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

TEST(MinMaxTest, HandlesSingleElement) {
    Data data;
    data.numbers = { 42.0 };
    InitializeCriticalSection(&data.cs);

    HANDLE hThread = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    WaitForSingleObject(hThread, INFINITE);

    EXPECT_DOUBLE_EQ(42.0, data.min);
    EXPECT_DOUBLE_EQ(42.0, data.max);

    DeleteCriticalSection(&data.cs);
    CloseHandle(hThread);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}