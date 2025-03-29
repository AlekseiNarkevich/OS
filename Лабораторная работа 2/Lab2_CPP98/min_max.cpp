#include <iostream>
#include <vector>
#include <windows.h>
#include "data.h"
DWORD WINAPI findMinMax(LPVOID param)
{
    Data* data = (Data*)param;
    if (data->numbers.empty()) return 0;
    data->min = data->numbers[0];
    data->max = data->numbers[0];
    for (size_t i = 0; i < data->numbers.size(); ++i)
    {
        if (data->numbers[i] < data->min) data->min = data->numbers[i];
        if (data->numbers[i] > data->max) data->max = data->numbers[i];
        Sleep(7);
    }
    EnterCriticalSection(&Data::cs);
    std::cout << "Минимум: " << data->min << ", Максимум: " << data->max << std::endl;
    LeaveCriticalSection(&Data::cs);
    return 0;
}