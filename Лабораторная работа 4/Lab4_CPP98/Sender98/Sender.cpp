#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "structs.h"
using namespace std;
int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "ru");
    if (argc < 3)
    {
        cerr << "Использование: Sender98 <файл> <ID>" << endl;
        return 1;
    }
    string binFl = argv[1];
    int threadId = atoi(argv[2]);
    cout << "Сендер стартовал с ID: " << threadId << endl;
    ostringstream ossTitle;
    ossTitle << "Sender ID: " << threadId;
    SetConsoleTitleA(ossTitle.str().c_str());
    HANDLE mutexFile = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "mutexFile");
    HANDLE FreeSlot = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "FreeSlot");
    HANDLE LockedSlot = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "LockedSlot");
    ostringstream ossEvent;
    ossEvent << "ReadyN" << threadId;
    HANDLE letsgoEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, ossEvent.str().c_str());
    SetEvent(letsgoEvent);
    string command;
    while (true)
    {
        cout << "Введите команду (s - отправить, q - выйти): ";
        cin >> command;
        if (command == "q") break;
        else if (command == "s")
        {
            string messageStr;
            cout << "Введите сообщение (макс. 20 символов): ";
            cin.ignore();
            getline(cin, messageStr);
            if (messageStr.length() >= 20) 
            {
                cout << "Ошибка: сообщение должно быть короче 20 символов! (Получено: " << messageStr.length() << ")\n";
                continue;
            }
            WaitForSingleObject(FreeSlot, INFINITE);
            WaitForSingleObject(mutexFile, INFINITE);
            ifstream file(binFl.c_str(), ios::binary);
            vector<SharedData> datamas;
            int lowestID = INT_MAX, maxID = -1;
            SharedData temp;
            while (file.read(reinterpret_cast<char*>(&temp), sizeof(temp)))
            {
                datamas.push_back(temp);
                if (temp.messageID < lowestID) lowestID = temp.messageID;
                if (temp.messageID > maxID) maxID = temp.messageID;
            }
            file.close();
            for (size_t i = 0; i < datamas.size(); ++i)
            {
                if (datamas[i].messageID == lowestID)
                {
                    strncpy(datamas[i].message, messageStr.c_str(), 19);
                    datamas[i].message[19] = '\0';
                    datamas[i].messageID = maxID + 1;
                    datamas[i].writerID = threadId;
                    break;
                }
            }
            ofstream out(binFl.c_str(), ios::binary | ofstream::trunc);
            for (size_t i = 0; i < datamas.size(); ++i)
            {
                out.write(reinterpret_cast<char*>(&datamas[i]), sizeof(SharedData));
            }
            out.close();
            ReleaseMutex(mutexFile);
            ReleaseSemaphore(LockedSlot, 1, NULL);
        }
        else
        {
            cout << "Неизвестная команда.\n";
        }
    }
    CloseHandle(mutexFile);
    CloseHandle(FreeSlot);
    CloseHandle(LockedSlot);
    CloseHandle(letsgoEvent);
    return 0;
}