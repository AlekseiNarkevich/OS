#include <iostream>
#include <vector>
#include <windows.h>
#include "data.h"
#include "min_max.h"
#include "average.h"
int main() 
{
    setlocale(LC_ALL, "ru");
    int size;
    std::cout << "������� ������ �������: ";
    std::cin >> size;
    std::vector<double> numbers(size);
    std::cout << "������� �������� �������: ";
    for (int i = 0; i < size; i++)
    {
        std::cin >> numbers[i];
    }
    Data data = {numbers, 0, 0, 0};
    HANDLE hMinMax = CreateThread(NULL, 0, findMinMax, &data, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, calculateAverage, &data, 0, NULL);
    if (!hMinMax || !hAverage) 
    {
        std::cerr << "������ �������� �������!" << std::endl;
        return 1;
    }
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);
    for (double& num : data.numbers)
    {
        if (num == data.min || num == data.max)
        {
            num = data.average;
        }
    }
    std::cout << "���������� ������: ";
    for (double num : data.numbers)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    if (hMinMax) CloseHandle(hMinMax);
    if (hAverage) CloseHandle(hAverage);
    return 0;
}
