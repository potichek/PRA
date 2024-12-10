# Powershell Remote Access (PRA)
PRA is a program (or Trojan virus) that allows access to the victim's Powershell. The program is in early access as its development has only recently begun. You can encounter a large number of bugs and nuances when using it, please write about it. Also, please use this program for educational purposes, do not use it to harm anyone.

## How does it work
We have 2 executable files. The first file is the client, and the second is the server. The client file must be located and run on the victim's computer. The server file must be located and run on the control computer. The client file will constantly try to connect to the server and when the connection is successfully established, the server will be able to send commands to the client, which it will execute through the victim's Powershell and send the result back to the server.

Now let's talk about additional elements of PRA. The executable file that must be opened by the victim is called a stub. The purpose of the stub is to recreate the client in the system that is stored directly in it. In order to "Put" the machine code into the stub, we need a packer. The packer can also encrypt the machine code of the client executable file and the stub can decrypt it, but these functions are not currently available in PRA. In addition to unpacking the client, the stub also adds it to Windows autoload.

## How to use
The first and most important thing you need to do for the program to work is to forward port 1488 (the program runs on it). Next, open the client.c file and enter your IP address in quotes in the line below:

```
sai.sin_addr.S_un.S_addr = inet_addr("your ip here or "127.0.0.1" if you are testing the program on a local computer");
```

After all the above, all you have to do is compile the client.c file. I use GCC for compilation and if you do the same, just run run.bat in the client folder. That's it! You can send your victim client.exe (you can rename it)

## Notes
Because this is a very early version there are a lot of shortcomings. Below are some of them.

- Unicode. In all the tests I've done, commands like "dir" output Unicode normally, but if we enter a command to output the contents of a file, for example "Get-Content", then Unicode may or may not be output normally. You have to experiment with the -Encoding parameter.
- Detection by Antivirus. The application is very easily recognized by antivirus, so if the victim's computer has an antivirus, it will easily delete PRA.

Everything written above will be corrected in future versions of PRA. [If you want to support the author and speed up the development process](https://www.donationalerts.com/r/potichek).