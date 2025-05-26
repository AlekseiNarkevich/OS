#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "data.h"
void findMinMax(Data* data)
{
    if (data->numbers.empty()) return;
    double local_min = data->numbers[0];
    double local_max = data->numbers[0];
    for (auto num : data->numbers) 
    {
        if (num < local_min) local_min = num;
        if (num > local_max) local_max = num;
        std::this_thread::sleep_for(std::chrono::milliseconds(7));
    }
    {
        std::lock_guard<std::mutex> lock(data->mtx);
        data->min = local_min;
        data->max = local_max;
    }
    {
        std::lock_guard<std::mutex> cout_lock(Data::cout_mtx);
        std::cout << "Минимум: " << data->min << ", Максимум: " << data->max << std::endl;
    }
}