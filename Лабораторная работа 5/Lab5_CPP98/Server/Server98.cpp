#define _CRT_SECURE_NO_WARNINGS
#include "structs.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;
string binFl;
int clKol = 0;
HANDLE* allDeadEvents = NULL;
HANDLE* YouReadNowEvents = NULL;
CRITICAL_SECTION recLocksCS;
struct RecordLockInfo
{
    bool isWriteLocked;
    int writerClientId;
    set<int> readerClientIds;
    RecordLockInfo() : isWriteLocked(false), writerClientId(-1) {}
};
map<int, RecordLockInfo> recLocks;
void DisplayEmployeeFile();
Employee FindEmployee(int employeeId);
bool ChangeEmployee(const Employee& empData);
DWORD WINAPI ClientHandlerThread(LPVOID lpParam);
struct ClientThreadArgs 
{
    HANDLE hPipe;
    ClientThreadArgs() : hPipe(INVALID_HANDLE_VALUE) {}
};
void CleanUp()
{
    if (allDeadEvents)
    {
        for (int i = 0; i < clKol; ++i)
        {
            if (allDeadEvents[i]) CloseHandle(allDeadEvents[i]);
        }
        delete[] allDeadEvents;
    }
    if (YouReadNowEvents)
    {
        for (int i = 0; i < clKol; ++i) 
        {
            if (YouReadNowEvents[i]) CloseHandle(YouReadNowEvents[i]);
        }
        delete[] YouReadNowEvents;
    }

    DeleteCriticalSection(&recLocksCS);
}
void DisplayEmployee(const Employee& emp)
{
    cout << "Сотрудник #" << emp.num << " - "
        << emp.name << " - "
        << emp.hours << " часов" << endl;
}
void DisplayEmployeeFile() 
{
    ifstream file(binFl.c_str(), ios::binary);
    if (!file.is_open())
    {
        cerr << "Не удалось открыть файл " << binFl << endl;
        return;
    }
    Employee data;
    cout << endl << "--- Содержимое файла сотрудников ---" << endl;
    while (file.read(reinterpret_cast<char*>(&data), sizeof(Employee)))
    {
        DisplayEmployee(data);
    }
    cout << "--------------------------------------" << endl << endl;
    file.close();
}
Employee FindEmployee(int employeeId) 
{
    ifstream file(binFl.c_str(), ios::binary);
    Employee data;
    data.num = -1;
    if (!file.is_open())
    {
        cerr << "Не удалось открыть файл " << binFl << endl;
        return data;
    }
    while (file.read(reinterpret_cast<char*>(&data), sizeof(Employee)))
    {
        if (data.num == employeeId)
        {
            file.close();
            return data;
        }
    }

    file.close();
    data.num = -1;
    return data;
}
bool ChangeEmployee(const Employee& empData)
{
    fstream file(binFl.c_str(), ios::binary | ios::in | ios::out);
    if (!file.is_open()) 
    {
        cerr << "Не удалось открыть файл " << binFl << endl;
        return false;
    }
    Employee tempEmp;
    streamoff recordPosition = -1;
    streamoff currentPosition = 0;
    while (file.read(reinterpret_cast<char*>(&tempEmp), sizeof(Employee)))
    {
        if (tempEmp.num == empData.num) 
        {
            recordPosition = currentPosition;
            break;
        }
        currentPosition = file.tellg();
    }
    if (recordPosition != -1) 
    {
        file.seekp(recordPosition, ios::beg);
        file.write(reinterpret_cast<const char*>(&empData), sizeof(Employee));
        file.close();
        return true;
    }
    file.close();
    return false;
}
int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    InitializeCriticalSection(&recLocksCS);
    cout << "Введите имя бинарного файла: ";
    cin >> binFl;
    int numEmpl;
    cout << "Введите количество сотрудников: ";
    cin >> numEmpl;
    ofstream outFile(binFl.c_str(), ios::binary | ios::trunc);
    if (!outFile.is_open()) 
    {
        cerr << "Не удалось создать файл " << binFl << endl;
        DeleteCriticalSection(&recLocksCS);
        return 1;
    }
    for (int i = 0; i < numEmpl; ++i)
    {
        Employee emp;
        emp.num = i;
        cout << "Введите имя сотрудника #" << i << ": ";
        cin >> emp.name;
        cout << "Введите количество часов для сотрудника #" << i << ": ";
        cin >> emp.hours;
        outFile.write(reinterpret_cast<char*>(&emp), sizeof(Employee));
    }
    outFile.close();
    DisplayEmployeeFile();
    cout << "Введите количество клиентов: ";
    cin >> clKol;
    allDeadEvents = new HANDLE[clKol];
    YouReadNowEvents = new HANDLE[clKol];
    for (int i = 0; i < clKol; ++i) 
    {
        char deadEventName[50];
        sprintf(deadEventName, "DeadEventClient%d", i);
        allDeadEvents[i] = CreateEvent(NULL, TRUE, FALSE, deadEventName);

        char readReadyEventName[50];
        sprintf(readReadyEventName, "YouCanReadNowClient%d", i);
        YouReadNowEvents[i] = CreateEvent(NULL, FALSE, FALSE, readReadyEventName);
    }
    vector<PROCESS_INFORMATION> clPi(clKol);
    vector<STARTUPINFO> clSi(clKol);
    for (int i = 0; i < clKol; ++i) 
    {
        ZeroMemory(&clSi[i], sizeof(STARTUPINFO));
        clSi[i].cb = sizeof(STARTUPINFO);
        ZeroMemory(&clPi[i], sizeof(PROCESS_INFORMATION));
        char commandLine[50];
        sprintf(commandLine, "Client.exe %d", i);
        if (!CreateProcess(NULL, commandLine, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE, NULL, NULL, &clSi[i], &clPi[i]))
        {
            cerr << "Не удалось создать процесс клиента #" << i << endl;
            continue;
        }
        cout << "Запущен процесс клиента #" << i << endl;
    }
    vector<HANDLE> clhThreadH;
    clhThreadH.reserve(clKol);
    for (int i = 0; i < clKol; ++i)
    {
        HANDLE hPipe = CreateNamedPipe(
            "\\\\.\\pipe\\employeeDataPipe",
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            sizeof(Message),
            sizeof(Message),
            0,
            NULL);
        if (hPipe == INVALID_HANDLE_VALUE)
        {
            cerr << "Не удалось создать именованный канал для клиента #" << i << endl;
            continue;
        }
        cout << "Ожидание подключения клиента #" << i << "..." << endl;
        if (ConnectNamedPipe(hPipe, NULL))
        {
            cout << "Клиент #" << i << " подключен" << endl;
            ClientThreadArgs* pArgs = new ClientThreadArgs();
            pArgs->hPipe = hPipe;
            HANDLE hThread = CreateThread(
                NULL,
                0,
                ClientHandlerThread,
                pArgs,
                0,
                NULL);
            if (hThread)
            {
                clhThreadH.push_back(hThread);
            }
            else 
            {
                cerr << "Не удалось создать поток для клиента #" << i << endl;
                delete pArgs;
            }
        }
        else
        {
            cerr << "Ошибка подключения клиента #" << i << endl;
            CloseHandle(hPipe);
        }
    }
    if (clKol > 0)
    {
        WaitForMultipleObjects(clKol, allDeadEvents, TRUE, INFINITE);
        cout << "Все клиенты завершили работу" << endl;
    }
    for (size_t i = 0; i < clhThreadH.size(); ++i)
    {
        if (clhThreadH[i])
        {
            WaitForSingleObject(clhThreadH[i], INFINITE);
            CloseHandle(clhThreadH[i]);
        }
    }
    DisplayEmployeeFile();
    for (int i = 0; i < clKol; ++i)
    {
        if (clPi[i].hProcess) CloseHandle(clPi[i].hProcess);
        if (clPi[i].hThread) CloseHandle(clPi[i].hThread);
    }
    CleanUp();
    cout << endl << "Нажмите Enter для выхода..." << endl;
    cin.ignore();
    cin.get();
    return 0;
}
DWORD WINAPI ClientHandlerThread(LPVOID lpParam)
{
    ClientThreadArgs* args = (ClientThreadArgs*)lpParam;
    HANDLE hPipe = args->hPipe;
    Message request, response;
    DWORD bytesRead, bytesWritten;
    bool clientActive = true;
    while (clientActive) 
    {
        if (!ReadFile(hPipe, &request, sizeof(Message), &bytesRead, NULL) || bytesRead == 0)
        {
            clientActive = false;
            continue;
        }
        cout << "Получен запрос типа " << request.type << " от клиента " << request.clientId << endl;
        response.clientId = request.clientId;
        response.employeeId = request.employeeId;
        EnterCriticalSection(&recLocksCS);
        RecordLockInfo& lockInfo = recLocks[request.employeeId];
        switch (request.type)
        {
        case READ_REQUEST:
            if (lockInfo.isWriteLocked)
            {
                response.type = BLOCK_RESPONSE;
            }
            else 
            {
                response.employee = FindEmployee(request.employeeId);
                if (response.employee.num == -1)
                {
                    response.type = FAIL_READ;
                }
                else 
                {
                    response.type = SUCCESS_READ;
                    lockInfo.readerClientIds.insert(request.clientId);
                }
            }
            break;
        case WRITE_REQUEST:
            if (lockInfo.isWriteLocked || !lockInfo.readerClientIds.empty())
            {
                response.type = BLOCK_RESPONSE;
            }
            else
            {
                response.employee = FindEmployee(request.employeeId);
                if (response.employee.num == -1) 
                {
                    response.type = FAIL_READ;
                }
                else
                {
                    response.type = SUCCESS_READ;
                    lockInfo.isWriteLocked = true;
                    lockInfo.writerClientId = request.clientId;
                }
            }
            break;
        case WRITE_REQUEST_READY:
            if (lockInfo.isWriteLocked && lockInfo.writerClientId == request.clientId)
            {
                if (ChangeEmployee(request.employee))
                {
                    response.type = SUCCESS;
                    response.employee = FindEmployee(request.employee.num);
                }
                else
                {
                    response.type = FAIL_READ;
                }
                lockInfo.isWriteLocked = false;
                lockInfo.writerClientId = -1;
            }
            else 
            {
                response.type = BLOCK_RESPONSE;
            }
            break;
        default:
            response.type = FAIL_READ;
            break;
        }
        LeaveCriticalSection(&recLocksCS);
        if (!WriteFile(hPipe, &response, sizeof(Message), &bytesWritten, NULL)) 
        {
            cerr << "Ошибка записи в канал для клиента " << request.clientId << endl;
            clientActive = false;
        }
        SetEvent(YouReadNowEvents[request.clientId]);
    }

    EnterCriticalSection(&recLocksCS);
    for (map<int, RecordLockInfo>::iterator it = recLocks.begin(); it != recLocks.end(); ++it)
    {
        RecordLockInfo& lockInfo = it->second;
        if (lockInfo.writerClientId == request.clientId)
        {
            lockInfo.isWriteLocked = false;
            lockInfo.writerClientId = -1;
        }
        lockInfo.readerClientIds.erase(request.clientId);
    }
    LeaveCriticalSection(&recLocksCS);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    delete args;
    return 0;
}