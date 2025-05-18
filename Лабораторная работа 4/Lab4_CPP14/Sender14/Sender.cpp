#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include "structs.h"
using namespace std;
int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "ru");
    if (argc < 3)
    {
        cerr << "Использование: Sender14 <имя_файла> <ID>" << endl;
        return 1;
    }
    string binFl = argv[1];
    int threadId = atoi(argv[2]);
    string title = "Sender ID: " + to_string(threadId);
    SetConsoleTitleA(title.c_str());
    cout << "Сендер стартовал с ID: " << threadId << endl;
    auto mutexFile = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "mutexFile");
    auto FreeSlot = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FreeSlot");
    auto LockedSlot = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "LockedSlot");
    string eventName = "ReadyN" + to_string(threadId);
    auto letsgoEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName.c_str());
    SetEvent(letsgoEvent);
    string command;
    SharedData data;
    while (true)
    {
        cout << "Введите команду (s - отправить, q - выйти): ";
        cin >> command;
        if (command == "q") break;
        if (command == "s")
        {
            string messageStr;
            cout << "Введите сообщение (макс. 20 символов): ";
            cin.ignore();
            getline(cin, messageStr);
            if (messageStr.length() >= 20)
            {
                cout << "Ошибка: сообщение должно быть короче 20 символов! (Получено: " << messageStr.length() << ")" << endl;
                continue;
            }
            WaitForSingleObject(FreeSlot, INFINITE);
            WaitForSingleObject(mutexFile, INFINITE);
            vector<SharedData> datamas;
            ifstream file(binFl, ios::binary);
            int lowestID = INT_MAX, maxID = -1;
            while (file.read(reinterpret_cast<char*>(&data), sizeof(data)))
            {
                datamas.emplace_back(data);
                if (data.messageID < lowestID) lowestID = data.messageID;
                if (data.messageID > maxID) maxID = data.messageID;
            }
            file.close();
            for (auto& el : datamas)
            {
                if (el.messageID == lowestID)
                {
                    strncpy_s(el.message, messageStr.c_str(), sizeof(el.message) - 1);
                    el.message[sizeof(el.message) - 1] = '\0';
                    el.messageID = maxID + 1;
                    el.writerID = threadId;
                    break;
                }
            }
            ofstream out(binFl, ios::binary | ofstream::trunc);
            for (const auto& el : datamas)
            {
                out.write(reinterpret_cast<const char*>(&el), sizeof(el));
            }
            out.close();
            ReleaseMutex(mutexFile);
            ReleaseSemaphore(LockedSlot, 1, nullptr);
        }
        else
        {
            cout << "Неизвестная команда." << endl;
        }
    }
    CloseHandle(mutexFile);
    CloseHandle(FreeSlot);
    CloseHandle(LockedSlot);
    CloseHandle(letsgoEvent);
    return 0;
}