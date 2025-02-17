#include "Employee.h"
#include <windows.h>

// Функция запуска процесса и ожидания завершения
bool runProcess(const std::string& command) {
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    char commandLine[256];
    strcpy_s(commandLine, command.c_str());

    if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Ошибка запуска процесса: " << command << "\n";
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

int main() {
    setlocale(LC_ALL, "ru");

    std::string binaryFile, reportFile;
    int recordCount;
    double rate;

    std::cout << "Введите имя бинарного файла: ";
    std::cin >> binaryFile;
    std::cout << "Введите количество записей: ";
    std::cin >> recordCount;

    if (!runProcess("Creator.exe " + binaryFile + " " + std::to_string(recordCount))) return 1;

    auto employees = readBinaryFile(binaryFile);
    printBinaryFile(employees);

    std::cout << "Введите имя файла отчета: ";
    std::cin >> reportFile;
    std::cout << "Введите оплату за час работы: ";
    std::cin >> rate;

    if (!runProcess("Reporter.exe " + binaryFile + " " + reportFile + " " + std::to_string(rate))) return 1;

    printTextFile(reportFile);
    return 0;
}
