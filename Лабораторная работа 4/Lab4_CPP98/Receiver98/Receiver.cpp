#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
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
    ofstream onF(binfl.c_str(), ios::binary);
    SharedData data;
    data.messageID = -1;
    data.writerID = -1;
    memset(data.message, 0, sizeof(data.message));
    for (int i = 0; i < kolZap; ++i)
    {
        onF.write(reinterpret_cast<char*>(&data), sizeof(data));
    }
    onF.close();
    int kolSenders;
    cout << "Введите количество отправителей: ";
    cin >> kolSenders;
    vector<STARTUPINFO> siv(kolSenders);
    vector<PROCESS_INFORMATION> piv(kolSenders);
    for (int i = 0; i < kolSenders; ++i)
    {
        ZeroMemory(&siv[i], sizeof(STARTUPINFO));
        siv[i].cb = sizeof(STARTUPINFO);
    }
    HANDLE* eventReady = new HANDLE[kolSenders];
    HANDLE mutexFile = CreateMutex(NULL, FALSE, "mutexFile");
    HANDLE FreeSlot = CreateSemaphore(NULL, kolZap, kolZap, "FreeSlot");
    HANDLE LockedSlot = CreateSemaphore(NULL, 0, kolZap, "LockedSlot");
    for (int i = 0; i < kolSenders; ++i)
    {
        ostringstream oss;
        oss << i;
        string eventName = "ReadyN" + oss.str();
        eventReady[i] = CreateEvent(NULL, FALSE, FALSE, eventName.c_str());
        string fullCommand = "Sender98.exe " + binfl + " " + oss.str();
        char* cmdLine = new char[fullCommand.size() + 1];
        strcpy(cmdLine, fullCommand.c_str());
        CreateProcess(
            NULL,
            cmdLine,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &siv[i],
            &piv[i]
        );
        delete[] cmdLine;
    }
    WaitForMultipleObjects(kolSenders, eventReady, TRUE, INFINITE);
    int messageID = -1;
    string com;
    while (true)
    {
        cout << "Введите команду (r - принять, q - выйти): ";
        cin >> com;
        if (com == "q") break;
        else if (com == "r")
        {
            WaitForSingleObject(LockedSlot, INFINITE);
            WaitForSingleObject(mutexFile, INFINITE);
            ifstream file(binfl.c_str(), ios::binary);
            ++messageID;
            bool found = false;
            SharedData temp;
            while (file.read(reinterpret_cast<char*>(&temp), sizeof(temp)))
            {
                if (temp.messageID == messageID)
                {
                    data = temp;
                    found = true;
                    if (strlen(data.message) >= 20)
                    {
                        cout << "Ошибка: получено некорректное сообщение (превышена длина)!\n";
                    }
                    else
                    {
                        cout << "Поток " << data.writerID << " написал: " << data.message << endl;
                    }
                    break;
                }
            }
            file.close();
            if (!found)
            {
                cout << "Сообщение с ID " << messageID << " не найдено!\n";
            }
            ReleaseMutex(mutexFile);
            ReleaseSemaphore(FreeSlot, 1, NULL);
        }
        else
        {
            cout << "Неизвестная команда.\n";
        }
    }
    for (int i = 0; i < kolSenders; ++i)
    {
        CloseHandle(piv[i].hThread);
        CloseHandle(piv[i].hProcess);
        CloseHandle(eventReady[i]);
    }
    CloseHandle(mutexFile);
    CloseHandle(FreeSlot);
    CloseHandle(LockedSlot);
    delete[] eventReady;
    return 0;
}