#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "structs.h"
using namespace std;
int main()
{
    setlocale(LC_ALL, "ru");
    string binfl;
    int kolZap;
    cout << "Введите имя бинарного файла: ";
    cin >> binfl;
    cout << "Введите количество записей: ";
    cin >> kolZap;
    ofstream onF(binfl, ios::binary);
    SharedData data{-1, -1, {}};
    for (int i = 0; i < kolZap; ++i)
    {
        onF.write(reinterpret_cast<const char*>(&data), sizeof(data));
    }
    onF.close();
    int kolSenders;
    cout << "Введите количество отправителей: ";
    cin >> kolSenders;
    auto mutexFile = CreateMutex(nullptr, FALSE, "mutexFile");
    auto FreeSlot = CreateSemaphore(nullptr, kolZap, kolZap, "FreeSlot");
    auto LockedSlot = CreateSemaphore(nullptr, 0, kolZap, "LockedSlot");
    vector<HANDLE> eventReady(kolSenders);
    vector<HANDLE> senderProcesses;
    STARTUPINFO si{sizeof(si)};
    PROCESS_INFORMATION pi{};
    for (int i = 0; i < kolSenders; ++i)
    {
        string evName = "ReadyN" + to_string(i);
        eventReady[i] = CreateEvent(nullptr, FALSE, FALSE, evName.c_str());
        string commandLine = "Sender14.exe " + binfl + " " + to_string(i);
        vector<char> cmdLine(commandLine.begin(), commandLine.end());
        cmdLine.push_back('\0');
        if (CreateProcess
        (
            nullptr,
            cmdLine.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            nullptr,
            &si,
            &pi
        ))
        {
            senderProcesses.emplace_back(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            cerr << "Ошибка запуска Sender.exe: " << GetLastError() << endl;
        }
    }
    WaitForMultipleObjects(static_cast<DWORD>(kolSenders), eventReady.data(), TRUE, INFINITE);
    cout << "Все Sender-процессы готовы!" << endl;
    int messageID = -1;
    string com;
    while (true)
    {
        cout << "Введите команду (r - принять, q - выйти): ";
        cin >> com;
        if (com == "q") break;
        if (com == "r")
        {
            WaitForSingleObject(LockedSlot, INFINITE);
            WaitForSingleObject(mutexFile, INFINITE);
            ifstream file(binfl, ios::binary);
            ++messageID;
            bool found = false;
            while (file.read(reinterpret_cast<char*>(&data), sizeof(data)))
            {
                if (data.messageID == messageID)
                {
                    found = true;
                    if (strnlen_s(data.message, 20) >= 20)
                    {
                        cout << "Ошибка: получено некорректное сообщение (превышена длина)!" << endl;
                    }
                    else
                    {
                        cout << "Принято сообщение от потока " << data.writerID << ": " << data.message << endl;
                    }
                    break;
                }
            }
            file.close();
            if (!found) 
            {
                cout << "Сообщение с ID " << messageID << " не найдено!" << endl;
            }

            ReleaseMutex(mutexFile);
            ReleaseSemaphore(FreeSlot, 1, nullptr);
        }
        else
        {
            cout << "Неизвестная команда." << endl;
        }
    }
    for (auto& h : eventReady) CloseHandle(h);
    for (auto& h : senderProcesses) CloseHandle(h);
    CloseHandle(mutexFile);
    CloseHandle(FreeSlot);
    CloseHandle(LockedSlot);
    return 0;
}