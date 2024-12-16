#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_SIZE 10000000

void to_powershell_command(LPWSTR str);
void execute_powershell_command(LPWSTR command, unsigned char *output, int *written_bytes);

LPCWSTR powershellPath = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
const wchar_t COMMAND_PREFIX[] = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe /C chcp 65001; ";
const unsigned int COMMAND_PREFIX_SIZE = (sizeof(COMMAND_PREFIX) / sizeof(wchar_t)) - 1;

void to_powershell_command(LPWSTR str)
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
    *(str + str_lenght + COMMAND_PREFIX_SIZE) = '\0';
    
    free(temp);
    return;
}

void execute_powershell_command(LPWSTR command, unsigned char *output, int *written_bytes)
{
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE command_stdout_read;
    HANDLE command_stdout_write;
    HANDLE command_stdin_read;
    HANDLE command_stdin_write;

    if (!CreatePipe(&command_stdin_read, &command_stdin_write, &sa, 0))
    {
        _putws(L"Pipe Error\n");
    }

    if (!CreatePipe(&command_stdout_read, &command_stdout_write, &sa, 0))
    {
        _putws(L"Pipe Error\n");
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdError = command_stdout_write;
    si.hStdOutput = command_stdout_write;
    si.hStdInput = command_stdin_read;
    si.wShowWindow = SW_HIDE;

    BOOL result = CreateProcess(
        powershellPath, 
        command, 
        NULL, 
        NULL, 
        TRUE, 
        0, 
        NULL, 
        NULL, 
        &si, 
        &pi
        );

    if (!result) _putws(L"Cannot create Proccess");
    
    CloseHandle(command_stdout_write);
    CloseHandle(command_stdin_read);

    unsigned char *buf = (unsigned char *) malloc(MAX_SIZE);
    unsigned char *coutput = (unsigned char *) malloc(MAX_SIZE);
    wchar_t *woutput = (wchar_t *) malloc(MAX_SIZE);

    memset(buf, 0, MAX_SIZE);
    memset(coutput, 0, MAX_SIZE);
    memset(woutput, 0, MAX_SIZE);

    int written = 0;
    DWORD readed = 0;
    
    while (1)
    {
        BOOL eof = ReadFile(command_stdout_read, buf, MAX_SIZE, &readed, NULL);
        buf[readed] = '\0';
        for(int i = 0; buf[i] != '\0'; i++)
        {
            coutput[written] = buf[i];
            written++;
        }
        if (!eof) break;
    }

    CloseHandle(command_stdout_read);
    CloseHandle(command_stdin_write);
    
    int written_in_output = 0;
    for (int i = 25; i <= written; i++)
    {
        output[written_in_output] = coutput[i];
        written_in_output++;
    }
    output[written_in_output - 1] = '\0';

    *written_bytes = written;

    free(buf);
    free(coutput);
    free(woutput);
    return;
}
