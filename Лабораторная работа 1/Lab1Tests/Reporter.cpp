#include "Employee.h"
#include <iomanip>

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc != 4) {
        std::cerr << "Использование: Reporter <имя бинарного файла> <имя отчета> <ставка>\n";
        return 1;
    }

    auto employees = readBinaryFile(argv[1]);
    if (employees.empty()) return 1;

    std::ofstream outFile(argv[2]);
    if (!outFile) {
        std::cerr << "Ошибка создания файла отчета.\n";
        return 1;
    }

    double rate = std::stod(argv[3]);
    outFile << "Отчет по файлу \"" << argv[1] << "\"\n";
    outFile << "ID\tИмя\tЧасы\tЗарплата\n";

    for (const auto& emp : employees) {
        outFile << emp.num << "\t" << emp.name << "\t" << emp.hours << "\t" << std::fixed << std::setprecision(2) << emp.hours * rate << "\n";
    }

    outFile.close();
    std::cout << "Отчет сохранен в " << argv[2] << "\n";
    return 0;
}
