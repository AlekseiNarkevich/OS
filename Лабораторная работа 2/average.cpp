#include <iostream>
#include <vector>
#include <windows.h>
struct Data 
{
    std::vector<double> numbers;
    double min, max, average;
};
DWORD WINAPI calculateAverage(LPVOID param) 
{
    Data* data = static_cast<Data*>(param);
    if (data->numbers.empty()) return 0;
    double sum = 0;
    for (double num : data->numbers) 
    {
        sum += num;
        Sleep(12);
    }
    data->average = sum / data->numbers.size();
    std::cout << "Среднее значение: " << data->average << std::endl;
    return 0;
}