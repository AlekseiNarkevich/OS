#include <iostream>
#include <iomanip>
#include "min_max.h"
DWORD WINAPI findMinMax(LPVOID param) 
{
    Data* data = (Data*)param;
    if (data->numbers.empty()) return 0;
    EnterCriticalSection(&data->cs);
    data->min = data->numbers[0];
    data->max = data->numbers[0];
    LeaveCriticalSection(&data->cs);
    for (double num : data->numbers) 
    {
        EnterCriticalSection(&data->cs);
        if (num < data->min)
        {
            data->min = num;
        }
        Sleep(7);
        if (num > data->max) 
        {
            data->max = num;
        }
        Sleep(7);
        LeaveCriticalSection(&data->cs);
    }
    EnterCriticalSection(&data->cs);
    std::cout << "Минимум: " << data->min << ", Максимум: " << data->max << std::endl;
    LeaveCriticalSection(&data->cs);
    return 0;
}