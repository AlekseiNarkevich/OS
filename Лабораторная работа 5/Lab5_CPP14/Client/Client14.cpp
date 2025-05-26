#define _CRT_SECURE_NO_WARNINGS
#include "structs.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <memory>
using namespace std;
HANDLE hNamedPipe = INVALID_HANDLE_VALUE;
HANDLE hNeedToReadEvent = nullptr;
HANDLE hICanReadEvent = nullptr;
HANDLE hIDeadEvent = nullptr;
int clientID = -1;
void CleanUpClient()
{
    if (hNeedToReadEvent) CloseHandle(hNeedToReadEvent);
    if (hICanReadEvent) CloseHandle(hICanReadEvent);
    if (hIDeadEvent) CloseHandle(hIDeadEvent);
    if (hNamedPipe != INVALID_HANDLE_VALUE) CloseHandle(hNamedPipe);
}
void DisplayEmployee(const Employee& emp) 
{
    cout << "��������� #" << emp.num << " - "
        << emp.name << " - "
        << emp.hours << " �����" << endl;
}
Employee ModifyEmployee(Employee data)
{
    cout << "������� ����� ��� ��� ���������� #" << data.num << " (�������: " << data.name << "): ";
    char newName[11];
    cin >> newName;
    strncpy(data.name, newName, 10);
    data.name[10] = '\0';
    cout << "������� ����� ���������� ����� ��� ���������� #" << data.num << " (�������: " << data.hours << "): ";
    cin >> data.hours;
    return data;
}
int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    if (argc < 2)
    {
        cerr << "�� ������ ID �������" << endl;
        return 1;
    }
    clientID = atoi(argv[1]);
    char title[100];
    sprintf(title, "������ #%d", clientID);
    SetConsoleTitle(title);
    cout << "������ #" << clientID << " �������" << endl;
    hNeedToReadEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "ReadEvent");
    char readNow[50];
    sprintf(readNow, "YouCanReadNowClient%d", clientID);
    hICanReadEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, readNow);
    char deadNow[50];
    sprintf(deadNow, "DeadEventClient%d", clientID);
    hIDeadEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, deadNow);
    hNamedPipe = CreateFile(
        "\\\\.\\pipe\\employeeDataPipe",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);
    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {
        cerr << "������ ����������� � ������" << endl;
        CleanUpClient();
        return 1;
    }
    Message message;
    DWORD dwBytes;
    bool running = true;
    while (running)
    {
        cout << "r - ������, s - ������, q - �����: ";
        char choice;
        cin >> choice;
        message.clientId = clientID;
        switch (choice)
        {
        case 'q':
            running = false;
            SetEvent(hIDeadEvent);
            break;
        case 'r':
            message.type = READ_REQUEST;
            cout << "������� ID ��� ������: ";
            cin >> message.employeeId;
            if (!WriteFile(hNamedPipe, &message, sizeof(Message), &dwBytes, nullptr))
            {
                cerr << "������ ������ � �����" << endl;
                running = false;
                break;
            }
            WaitForSingleObject(hICanReadEvent, INFINITE);
            ReadFile(hNamedPipe, &message, sizeof(Message), &dwBytes, nullptr);
            if (message.type == SUCCESS_READ) 
            {
                DisplayEmployee(message.employee);
            }
            else if (message.type == FAIL_READ) 
            {
                cout << "��������� �� ������" << endl;
            }
            else if (message.type == BLOCK_RESPONSE)
            {
                cout << "������ �������������" << endl;
            }
            break;
        case 's':
            message.type = WRITE_REQUEST;
            cout << "������� ID ���������� ��� ���������: ";
            cin >> message.employeeId;
            if (!WriteFile(hNamedPipe, &message, sizeof(Message), &dwBytes, nullptr)) 
            {
                cerr << "������ ������ � �����" << endl;
                running = false;
                break;
            }
            WaitForSingleObject(hICanReadEvent, INFINITE);
            ReadFile(hNamedPipe, &message, sizeof(Message), &dwBytes, nullptr);
            if (message.type == SUCCESS_READ) 
            {
                cout << "������� ������:" << endl;
                DisplayEmployee(message.employee);
                message.employee = ModifyEmployee(message.employee);
                message.type = WRITE_REQUEST_READY;
                message.clientId = clientID;
                if (!WriteFile(hNamedPipe, &message, sizeof(Message), &dwBytes, nullptr))
                {
                    cerr << "������ ������ � �����" << endl;
                    running = false;
                    break;
                }
                WaitForSingleObject(hICanReadEvent, INFINITE);
                ReadFile(hNamedPipe, &message, sizeof(Message), &dwBytes, nullptr);
                if (message.type == SUCCESS)
                {
                    cout << "������ ��������� �� �������:" << endl;
                    DisplayEmployee(message.employee);
                }
                else 
                {
                    cout << "������ ����������" << endl;
                }
            }
            else if (message.type == FAIL_READ)
            {
                cout << "��������� �� ������" << endl;
            }
            else if (message.type == BLOCK_RESPONSE)
            {
                cout << "������ �������������" << endl;
            }
            break;
        default:
            cout << "�������� �����" << endl;
            break;
        }
    }
    CleanUpClient();
    return 0;
}