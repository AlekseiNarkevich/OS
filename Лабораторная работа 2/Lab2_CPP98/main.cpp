#include <iostream>
#include <vector>
#include <windows.h>
#include <cassert>
#include "data.h"
#include "min_max.h"
#include "average.h"
int main()
{
    InitializeCriticalSection(&Data::cs);
    setlocale(LC_ALL, "Russian");
    int size;
    std::cout << "Размер массива: ";
    std::cin >> size;
    std::vector<double> numbers;
    std::cout << "Введите элементы: ";
    for (int i = 0; i < size; ++i)
    {
        double num;
        std::cin >> num;
        numbers.push_back(num);
    }
    Data data;
    data.numbers = numbers;
    HANDLE hMinMax = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    assert(hMinMax != NULL);
    HANDLE hAverage = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    assert(hAverage != NULL);
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);
    for (size_t i = 0; i < data.numbers.size(); ++i)
    {
        if (data.numbers[i] == data.min || data.numbers[i] == data.max)
        {
            data.numbers[i] = data.average;
        }
    }
    EnterCriticalSection(&Data::cs);
    std::cout << "Измененный массив: ";
    for (size_t i = 0; i < data.numbers.size(); ++i)
    {
        std::cout << data.numbers[i] << " ";
    }
    std::cout << std::endl;
    LeaveCriticalSection(&Data::cs);
    CloseHandle(hMinMax);
    CloseHandle(hAverage);
    DeleteCriticalSection(&Data::cs);
    return 0;
}