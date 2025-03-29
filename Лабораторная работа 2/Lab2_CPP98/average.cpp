#include <iostream>
#include <vector>
#include <windows.h>
#include "data.h"
DWORD WINAPI calculateAverage(LPVOID param)
{
    Data* data = (Data*)param;
    if (data->numbers.empty()) return 0;
    double sum = 0;
    for (size_t i = 0; i < data->numbers.size(); ++i)
    {
        sum += data->numbers[i];
        Sleep(12);
    }
    data->average = sum / data->numbers.size();
    EnterCriticalSection(&Data::cs);
    std::cout << "Среднее значение: " << data->average << std::endl;
    LeaveCriticalSection(&Data::cs);
    return 0;
}