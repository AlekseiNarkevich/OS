#include "Employee.h"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc != 3) {
        std::cerr << "Использование: Creator <имя файла> <количество записей>\n";
        return 1;
    }

    std::ofstream outFile(argv[1], std::ios::binary);
    if (!outFile) {
        std::cerr << "Ошибка открытия файла.\n";
        return 1;
    }

    int recordCount = std::stoi(argv[2]);
    std::vector<Employee> employees(recordCount);

    for (int i = 0; i < recordCount; i++) {
        std::cout << "Введите ID, имя и отработанные часы для сотрудника " << i + 1 << ": ";
        std::cin >> employees[i].num >> employees[i].name >> employees[i].hours;
    }

    outFile.write(reinterpret_cast<char*>(employees.data()), recordCount * sizeof(Employee));
    outFile.close();
    std::cout << "Файл " << argv[1] << " успешно создан.\n";
    return 0;
}