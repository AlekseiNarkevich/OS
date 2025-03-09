#ifndef DATA_H
#define DATA_H
#include <vector>
#include <windows.h>
struct Data 
{
    std::vector<double> numbers;
    double min, max;           
    double average;            
    CRITICAL_SECTION cs;
};
#endif