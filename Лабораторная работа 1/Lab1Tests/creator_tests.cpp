#include <windows.h>
#include <gtest/gtest.h>
#include "Employee.h"
#include <fstream>
#include <sstream>

TEST(CreatorTest, CreatesBinaryFile) {
    std::string filename = "test_creator.bin";  // Имя выходного файла
    std::string inputFile = "test_input.txt";   // Имя входного файла
    std::string command = "Creator.exe " + filename + " 2 " + inputFile;  // Команда с 4 аргументами

    // Создаем входной файл с данными
    std::ofstream inFile(inputFile);
    inFile << "1 Alice 40\n";  // Первая запись
    inFile << "2 Bob 30.5\n";  // Вторая запись
    inFile.close();

    // Запускаем Creator
    testing::internal::CaptureStdout();
    system(command.c_str());
    std::string output = testing::internal::GetCapturedStdout();

    // Проверяем, что файл создан
    std::ifstream outFile(filename, std::ios::binary);
    ASSERT_TRUE(outFile.is_open()) << "Файл не был создан: " << filename;

    // Читаем файл
    Employee emp1, emp2;
    outFile.read(reinterpret_cast<char*>(&emp1), sizeof(Employee));
    outFile.read(reinterpret_cast<char*>(&emp2), sizeof(Employee));
    outFile.close();

    // Проверяем содержимое
    EXPECT_EQ(emp1.num, 1);
    EXPECT_STREQ(emp1.name, "Alice");
    EXPECT_DOUBLE_EQ(emp1.hours, 40.0);
    EXPECT_EQ(emp2.num, 2);
    EXPECT_STREQ(emp2.name, "Bob");
    EXPECT_DOUBLE_EQ(emp2.hours, 30.5);

    // Удаляем временные файлы
    std::remove(filename.c_str());
    std::remove(inputFile.c_str());
}