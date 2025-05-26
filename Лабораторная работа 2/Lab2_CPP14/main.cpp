#include <iostream>
#include <vector>
#include <thread>
#include "data.h"
#include "min_max.h"
#include "average.h"
int main() 
{
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
    std::thread minMaxThread(findMinMax, &data);
    std::thread averageThread(calculateAverage, &data);
    minMaxThread.join();
    averageThread.join();
    {
        std::lock_guard<std::mutex> lock(data.mtx);
        for (auto& num : data.numbers) {
            if (num == data.min || num == data.max)
            {
                num = data.average;
            }
        }
    }
    std::cout << "Измененный массив: ";
    for (const auto& num : data.numbers)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    return 0;
}