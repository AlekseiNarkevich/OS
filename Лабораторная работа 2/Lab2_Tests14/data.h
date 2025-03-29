#ifndef DATA_H
#define DATA_H
#include <vector>
#include <mutex>
struct Data 
{
    std::vector<double> numbers;
    double min = 0;
    double max = 0;
    double average = 0;
    mutable std::mutex mtx;
    static std::mutex cout_mtx;
};
#endif