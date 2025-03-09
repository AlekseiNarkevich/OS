#include <iostream>
#include <vector>
#include <windows.h>
#include <iomanip>
#include "data.h"
#include "min_max.h"
#include "average.h"
int main()
{
    setlocale(LC_ALL, "Russian");
    Data data;
    InitializeCriticalSection(&data.cs);
    int size;
    std::cout << "Введите размер массива: ";
    std::cin >> size;
    if (size <= 0)
    {
        std::cerr << "Ошибка: размер должен быть положительным!" << std::endl;
        DeleteCriticalSection(&data.cs);
        return 1;
    }
    data.numbers.resize(size);
    std::cout << "Введите элементы массива (double): ";
    for (int i = 0; i < size; ++i) 
    {
        std::cin >> data.numbers[i];
    }
    HANDLE hThreads[2];
    hThreads[0] = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    if (!hThreads[0] || !hThreads[1]) 
    {
        std::cerr << "Ошибка создания потоков!" << std::endl;
        DeleteCriticalSection(&data.cs);
        return 1;
    }
    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);
    EnterCriticalSection(&data.cs);
    for (double& num : data.numbers)
    {
        if (num == data.min || num == data.max)
        {
            num = data.average;
        }
    }
    std::cout << "Измененный массив: ";
    for (double num : data.numbers) 
    {
        std::cout << std::fixed << std::setprecision(2) << num << " ";
    }
    std::cout << std::endl;
    LeaveCriticalSection(&data.cs);
    DeleteCriticalSection(&data.cs);
    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);
    return 0;
}