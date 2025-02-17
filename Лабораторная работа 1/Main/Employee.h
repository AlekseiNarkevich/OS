#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Структура сотрудника
struct Employee {
    int num;
    char name[10];
    double hours;
};

// Функции для работы с файлами
std::vector<Employee> readBinaryFile(const std::string& filename);
void printBinaryFile(const std::vector<Employee>& employees);
void printTextFile(const std::string& filename);

#endif // EMPLOYEE_H
