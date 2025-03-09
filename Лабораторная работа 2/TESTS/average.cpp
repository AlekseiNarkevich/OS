#include <iostream>
#include <iomanip>
#include "average.h"
DWORD WINAPI calculateAverage(LPVOID param) 
{
    Data* data = (Data*)param;
    if (data->numbers.empty()) return 0;
    double sum = 0;
    for (double num : data->numbers)
    {
        EnterCriticalSection(&data->cs);
        sum += num;
        LeaveCriticalSection(&data->cs);
        Sleep(12);
    }
    EnterCriticalSection(&data->cs);
    data->average = sum / data->numbers.size();
    std::cout << "Среднее значение: " << std::fixed << std::setprecision(5) << data->average << std::endl;
    LeaveCriticalSection(&data->cs);
    return 0;
}