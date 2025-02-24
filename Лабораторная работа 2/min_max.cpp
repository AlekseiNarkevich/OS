#include <iostream>
#include <vector>
#include <windows.h>
struct Data
{
    std::vector<double> numbers;
    double min, max, average;
};
DWORD WINAPI findMinMax(LPVOID param) 
{
    Data* data = static_cast<Data*>(param);
    if (data->numbers.empty()) return 0;
    data->min = data->numbers[0];
    data->max = data->numbers[0];
    for (double num : data->numbers) 
    {
        if (num < data->min) data->min = num;
        if (num > data->max) data->max = num;
        Sleep(7);
    }
    std::cout << "Минимум: " << data->min << ", Максимум: " << data->max << std::endl;
    return 0;
}