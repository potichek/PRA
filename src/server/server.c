#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#define MAX_SIZE 200000

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    _putws(L"The author of this program: Potichek.\nGithub: https://github.com/potichek\nDiscord: https://discord.gg/wE5SftnWct");

    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);

    SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN sai;
    
    memset(&sai, 0, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(1488);

    bind(soc, (const struct sockaddr *) &sai, sizeof(sai));
    listen(soc, 100);

    wchar_t dataw[MAX_SIZE];
    char datac[MAX_SIZE];

    memset(dataw, 0, sizeof(dataw));
    memset(datac, 0, sizeof(datac));
    SOCKET client_soc;
    SOCKADDR_IN client_addr;
    int client_addr_size = sizeof(client_addr);

    while (client_soc = accept(soc, (struct sockaddr *) &client_addr, &client_addr_size))
    {
        _putws(L"Connected!\n");

        while (1)
        {
            _putws(L"Enter command: ");
            fgetws(dataw, MAX_SIZE, stdin);

            int nibites = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t *) dataw, -1, NULL, 0, NULL, NULL);
            int iconverted = WideCharToMultiByte(
                CP_UTF8, 0, (const wchar_t *) dataw, -1, datac, nibites, NULL, NULL);
            send(client_soc, datac, iconverted, 0);

            recv(client_soc, datac, sizeof(datac), 0);
            int nobites = MultiByteToWideChar(CP_UTF8, 0, datac, -1, NULL, 0);
            int oconverted = MultiByteToWideChar(
                CP_UTF8, 0, datac, -1, dataw, nobites);
            
            _putws(dataw);
        }
    }

    return 0;
}