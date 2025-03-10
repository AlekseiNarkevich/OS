#include <gtest/gtest.h>
#include "Employee.h"
#include <fstream>

// Тест для функции readBinaryFile
TEST(EmployeeTest, ReadBinaryFile) {
    // Создаем временный бинарный файл
    std::string filename = "test_employees.bin";
    std::ofstream outFile(filename, std::ios::binary);
    Employee emp1 = { 1, "Alice", 40.0 };
    Employee emp2 = { 2, "Bob", 30.5 };
    outFile.write(reinterpret_cast<char*>(&emp1), sizeof(Employee));
    outFile.write(reinterpret_cast<char*>(&emp2), sizeof(Employee));
    outFile.close();

    // Читаем файл
    auto employees = readBinaryFile(filename);
    ASSERT_EQ(employees.size(), 2);
    EXPECT_EQ(employees[0].num, 1);
    EXPECT_STREQ(employees[0].name, "Alice");
    EXPECT_DOUBLE_EQ(employees[0].hours, 40.0);
    EXPECT_EQ(employees[1].num, 2);
    EXPECT_STREQ(employees[1].name, "Bob");
    EXPECT_DOUBLE_EQ(employees[1].hours, 30.5);

    // Удаляем временный файл
    std::remove(filename.c_str());
}

// Тест для функции printBinaryFile
TEST(EmployeeTest, PrintBinaryFile) {
    std::vector<Employee> employees = {
        {1, "Alice", 40.0},
        {2, "Bob", 30.5}
    };

    // Перенаправляем вывод в строку
    testing::internal::CaptureStdout();
    printBinaryFile(employees);
    std::string output = testing::internal::GetCapturedStdout();

    // Проверяем вывод
    EXPECT_NE(output.find("1\tAlice\t40"), std::string::npos);
    EXPECT_NE(output.find("2\tBob\t30.5"), std::string::npos);
}

// Тест для функции printTextFile
TEST(EmployeeTest, PrintTextFile) {
    // Создаем временный текстовый файл
    std::string filename = "test_report.txt";
    std::ofstream outFile(filename);
    outFile << "Отчет по файлу \"test_employees.bin\"\n";
    outFile << "ID\tИмя\tЧасы\tЗарплата\n";
    outFile << "1\tAlice\t40\t1600.00\n";
    outFile << "2\tBob\t30.5\t1220.00\n";
    outFile.close();

    // Перенаправляем вывод в строку
    testing::internal::CaptureStdout();
    printTextFile(filename);
    std::string output = testing::internal::GetCapturedStdout();

    // Проверяем вывод
    EXPECT_NE(output.find("1\tAlice\t40\t1600.00"), std::string::npos);
    EXPECT_NE(output.find("2\tBob\t30.5\t1220.00"), std::string::npos);

    // Удаляем временный файл
    std::remove(filename.c_str());
}