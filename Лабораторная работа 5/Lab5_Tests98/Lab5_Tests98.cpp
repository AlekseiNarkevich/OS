#define _CRT_SECURE_NO_WARNINGS
#include "gtest/gtest.h"
#include "structs.h"
#include <windows.h>
#include <fstream>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>
using namespace std;
string binFl;
CRITICAL_SECTION recLocksCS;
void DisplayEmployee(const Employee& emp)
{
    cout << "Сотрудник #" << emp.num << " - "
        << emp.name << " - "
        << emp.hours << " часов" << endl;
}
Employee FindEmployee(int employeeId) 
{
    ifstream file(binFl.c_str(), ios::binary);
    Employee data;
    data.num = -1;
    if (!file.is_open())
    {
        cerr << "Не удалось открыть файл " << binFl << endl;
        return data;
    }
    while (file.read(reinterpret_cast<char*>(&data), sizeof(Employee)))
    {
        if (data.num == employeeId)
        {
            file.close();
            return data;
        }
    }
    file.close();
    data.num = -1;
    return data;
}
bool ChangeEmployee(const Employee& empData)
{
    fstream file(binFl.c_str(), ios::binary | ios::in | ios::out);
    if (!file.is_open()) 
    {
        cerr << "Не удалось открыть файл " << binFl << endl;
        return false;
    }
    Employee tempEmp;
    streamoff recordPosition = -1;
    streamoff currentPosition = 0;
    while (file.read(reinterpret_cast<char*>(&tempEmp), sizeof(Employee)))
    {
        if (tempEmp.num == empData.num)
        {
            recordPosition = currentPosition;
            break;
        }
        currentPosition = file.tellg();
    }
    if (recordPosition != -1)
    {
        file.seekp(recordPosition, ios::beg);
        file.write(reinterpret_cast<const char*>(&empData), sizeof(Employee));
        file.close();
        return true;
    }
    file.close();
    return false;
}
void CreateTestFile(const string& filename, const vector<Employee>& employees) {
    ofstream outFile(filename.c_str(), ios::binary | ios::trunc);
    for (vector<Employee>::const_iterator it = employees.begin(); it != employees.end(); ++it) {
        outFile.write(reinterpret_cast<const char*>(&(*it)), sizeof(Employee));
    }
    outFile.close();
}
void DeleteTestFile(const string& filename)
{
    remove(filename.c_str());
}
TEST(StructsTest, EmployeeStruct) 
{
    Employee emp;
    emp.num = 1;
    strncpy(emp.name, "Test", 10);
    emp.hours = 40.5;
    EXPECT_EQ(emp.num, 1);
    EXPECT_STREQ(emp.name, "Test");
    EXPECT_DOUBLE_EQ(emp.hours, 40.5);
}
TEST(StructsTest, MessageStruct) 
{
    Message msg;
    msg.type = READ_REQUEST;
    msg.employeeId = 1;
    msg.clientId = 1;
    msg.employee.num = 1;
    strncpy(msg.employee.name, "Test", 10);
    msg.employee.hours = 40.5;
    EXPECT_EQ(msg.type, READ_REQUEST);
    EXPECT_EQ(msg.employeeId, 1);
    EXPECT_EQ(msg.clientId, 1);
    EXPECT_EQ(msg.employee.num, 1);
    EXPECT_STREQ(msg.employee.name, "Test");
    EXPECT_DOUBLE_EQ(msg.employee.hours, 40.5);
}
TEST(FileFunctionsTest, FindEmployee)
{
    const string testFile = "test_employees.bin";
    Employee emp1 = { 1, "Alice", 35.0 };
    Employee emp2 = { 2, "Bob", 40.0 };
    Employee emp3 = { 3, "Charlie", 45.0 };
    vector<Employee> testEmployees;
    testEmployees.push_back(emp1);
    testEmployees.push_back(emp2);
    testEmployees.push_back(emp3);
    CreateTestFile(testFile, testEmployees);
    binFl = testFile;
    Employee emp = FindEmployee(2);
    EXPECT_EQ(emp.num, 2);
    EXPECT_STREQ(emp.name, "Bob");
    EXPECT_DOUBLE_EQ(emp.hours, 40.0);
    emp = FindEmployee(4);
    EXPECT_EQ(emp.num, -1);
    DeleteTestFile(testFile);
}
TEST(FileFunctionsTest, ChangeEmployee)
{
    const string testFile = "test_employees.bin";
    Employee emp1 = { 1, "Alice", 35.0 };
    Employee emp2 = { 2, "Bob", 40.0 };
    vector<Employee> testEmployees;
    testEmployees.push_back(emp1);
    testEmployees.push_back(emp2);
    CreateTestFile(testFile, testEmployees);
    binFl = testFile;
    Employee modified = { 2, "Robert", 42.5 };
    bool result = ChangeEmployee(modified);
    EXPECT_TRUE(result);
    Employee emp = FindEmployee(2);
    EXPECT_EQ(emp.num, 2);
    EXPECT_STREQ(emp.name, "Robert");
    EXPECT_DOUBLE_EQ(emp.hours, 42.5);
    Employee nonExistent = { 3, "Dave", 50.0 };
    result = ChangeEmployee(nonExistent);
    EXPECT_FALSE(result);

    DeleteTestFile(testFile);
}
TEST(DisplayTest, DisplayEmployee)
{
    Employee emp;
    emp.num = 1;
    strncpy(emp.name, "Alice", 10);
    emp.hours = 35.5;
    EXPECT_NO_THROW(DisplayEmployee(emp));
}
int main(int argc, char** argv) 
{
    InitializeCriticalSection(&recLocksCS);
    testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    DeleteCriticalSection(&recLocksCS);
    return result;
}