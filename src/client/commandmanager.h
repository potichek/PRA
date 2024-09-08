#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_SIZE 1000

void to_powershell_command(wchar_t *str);
void execute_powershell_command(wchar_t *command);

const wchar_t COMMAND_PREFIX[] = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe /C ";
const unsigned int COMMAND_PREFIX_SIZE = (sizeof(COMMAND_PREFIX) / sizeof(wchar_t)) - 1;
const wchar_t COMMAND_SUFFIX[MAX_SIZE] = L" | Out-File -FilePath \"output.txt\"";
const unsigned int COMMAND_SUFFIX_SIZE = (sizeof(COMMAND_SUFFIX) / sizeof(wchar_t));

void to_powershell_command(wchar_t *str)
{ 
    wchar_t *temp = (wchar_t *) malloc(MAX_SIZE);

    unsigned int str_lenght;
    for (str_lenght = 0; *(str + str_lenght) != '\0'; str_lenght++)
    {
        *(temp + str_lenght) = *(str + str_lenght);
    }

    *(temp + str_lenght) = '\0';

    for (unsigned int i = 0; i < COMMAND_PREFIX_SIZE; i++)
    {
        *(str + i) = COMMAND_PREFIX[i];
    }

    
    for (unsigned int i = 0; i < str_lenght; i++)
    {
        *(str + i + COMMAND_PREFIX_SIZE) = *(temp + i);
    }

    free(temp);

    str_lenght--;
    for (unsigned int i = 0; i < COMMAND_SUFFIX_SIZE; i++)
    {
        *(str + COMMAND_PREFIX_SIZE + str_lenght + i) = COMMAND_SUFFIX[i];
    }

    return;
}

void execute_powershell_command(wchar_t *command)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    LPCWSTR powershellPath = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";

    BOOL result = CreateProcessW(powershellPath, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    DWORD errCode = GetLastError();
    if (!result)
    {
        _putws(L"Cannot execute powershell command");
    }

    return;
}