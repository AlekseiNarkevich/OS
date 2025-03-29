#ifndef DATA_H
#define DATA_H
#include <vector>
#include <windows.h>
struct Data
{
    std::vector<double> numbers;
    double min;
    double max;
    double average;
    static CRITICAL_SECTION cs;
};
#endif