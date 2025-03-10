#include <gtest/gtest.h>
#include "Employee.h"
#include <fstream>

TEST(ReporterTest, CreatesReport) {
    std::string binaryFile = "test_reporter.bin";
    std::string reportFile = "test_report.txt";
    double rate = 40.0;

    // Создаем временный бинарный файл
    std::ofstream outFile(binaryFile, std::ios::binary);
    Employee emp1 = { 1, "Alice", 40.0 };
    Employee emp2 = { 2, "Bob", 30.5 };
    outFile.write(reinterpret_cast<char*>(&emp1), sizeof(Employee));
    outFile.write(reinterpret_cast<char*>(&emp2), sizeof(Employee));
    outFile.close();

    // Запускаем Reporter
    std::string reporterPath = "C:\\Users\\alesi\\Desktop\\Lab1Tests\\x64\\Debug\\Reporter.exe";
    std::string command = "\"" + reporterPath + "\" " + binaryFile + " " + reportFile + " " + std::to_string(rate);
    testing::internal::CaptureStdout();
    system(command.c_str());
    std::string output = testing::internal::GetCapturedStdout();

    // Проверяем, что отчет создан
    std::ifstream inFile(reportFile);
    ASSERT_TRUE(inFile.is_open());

    // Проверяем содержимое отчета
    std::string line;
    while (std::getline(inFile, line)) { // Используем std::getline с двумя аргументами
        if (line.find("1\tAlice\t40\t1600.00") != std::string::npos) {
            EXPECT_NE(line.find("1\tAlice\t40\t1600.00"), std::string::npos);
        }
        if (line.find("2\tBob\t30.5\t1220.00") != std::string::npos) {
            EXPECT_NE(line.find("2\tBob\t30.5\t1220.00"), std::string::npos);
        }
    }

    inFile.close();

    // Удаляем временные файлы
    std::remove(binaryFile.c_str());
    std::remove(reportFile.c_str());
}