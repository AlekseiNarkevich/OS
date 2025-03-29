#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "data.h"
void calculateAverage(Data* data)
{
    if (data->numbers.empty()) return;
    double sum = 0;
    for (auto num : data->numbers)
    {
        sum += num;
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
    const double avg = sum / data->numbers.size();
    {
        std::lock_guard<std::mutex> lock(data->mtx);
        data->average = avg;
    }
    {
        std::lock_guard<std::mutex> cout_lock(Data::cout_mtx);
        std::cout << "Среднее значение: " << data->average << std::endl;
    }
}