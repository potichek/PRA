#ifndef UNICODE
#define UNICODE
#endif 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void get_pe_data(LPCSTR name, LPVOID data);
DWORD get_file_size(LPCSTR name);
void create_new_pe(LPCWSTR name, LPVOID data, DWORD size);
void add_section(LPVOID data, LPVOID section_data, DWORD section_size, DWORD stub_size);
DWORD align(DWORD value, DWORD alignment);

int main(int argc, char *argv[])
{

   if (argc < 3)
   {
      printf("Specify the name of the stub and dummy\n");
      printf("<dummy> <stub>\n");
      return 1;
   }

   DWORD packed_size = get_file_size(argv[1]);
   LPVOID packed_data = malloc(packed_size);
   get_pe_data(argv[1], packed_data);

   DWORD stub_size = get_file_size(argv[2]);
   LPVOID new_pe_data = malloc(stub_size + packed_size);
   get_pe_data(argv[2], new_pe_data);
   add_section(new_pe_data, packed_data, packed_size, stub_size);

   create_new_pe(L"newexe.exe", new_pe_data, (stub_size + packed_size));
   return 0;
}

DWORD align(DWORD value, DWORD alignment)
{
   return value + ((value % alignment == 0) ? 0 : alignment - (value % alignment));
}

DWORD get_file_size(LPCSTR name)
{
   HANDLE file = CreateFileA(
      name,
      GENERIC_READ,
      FILE_SHARE_READ, 
      NULL, 
      OPEN_EXISTING, 
      FILE_ATTRIBUTE_NORMAL,
      NULL
      );

   if (file == INVALID_HANDLE_VALUE) {
      printf("Last error: %d\n", GetLastError());
      return 0;
   }

   return GetFileSize(file, NULL);
}

void get_pe_data(LPCSTR name, LPVOID data)
{
   HANDLE file = CreateFileA(
      name,
      GENERIC_READ,
      FILE_SHARE_READ, 
      NULL, 
      OPEN_EXISTING, 
      FILE_ATTRIBUTE_NORMAL,
      NULL
      );

   if (file == INVALID_HANDLE_VALUE) {
      printf("Last error: %d\n", GetLastError());
      return;
   }

   DWORD file_size = GetFileSize(file, NULL);
   DWORD readed_bytes = 0;

   int res = ReadFile(
      file,
      data,
      file_size,
      &readed_bytes,
      NULL
   );

   CloseHandle(file);

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

   return;
}

void add_section(LPVOID data, LPVOID section_data, DWORD section_size, DWORD stub_size)
{
   BYTE *pointer = (BYTE *) data;
   PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER) pointer;

   pointer += dos_header->e_lfanew;
   PIMAGE_NT_HEADERS64 nt_headers = (PIMAGE_NT_HEADERS64) pointer;

   DWORD section_alignment = nt_headers->OptionalHeader.SectionAlignment;
   DWORD file_alignment = nt_headers->OptionalHeader.FileAlignment;

   nt_headers->FileHeader.NumberOfSections++;
   WORD num_of_sections = nt_headers->FileHeader.NumberOfSections;

   pointer = (BYTE *) &(nt_headers->OptionalHeader) + nt_headers->FileHeader.SizeOfOptionalHeader;
   PIMAGE_SECTION_HEADER section_table = (PIMAGE_SECTION_HEADER) pointer;

   PIMAGE_SECTION_HEADER previous_section = &section_table[num_of_sections - 2];
   PIMAGE_SECTION_HEADER new_section = &section_table[num_of_sections - 1];

   DWORD virtual_offset = align(previous_section->VirtualAddress + previous_section->Misc.VirtualSize, section_alignment);
   DWORD raw_offset = align(previous_section->SizeOfRawData + previous_section->PointerToRawData, file_alignment);
   unsigned long long temp = (unsigned long long) raw_offset;
   pointer = (BYTE *) data + temp;


   memcpy(new_section->Name, ".packed", 8);
   new_section->Misc.VirtualSize = section_size;
   new_section->VirtualAddress = virtual_offset;
   new_section->SizeOfRawData = align(section_size + sizeof(size_t), file_alignment);
   new_section->PointerToRawData = raw_offset;
   new_section->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

   nt_headers->OptionalHeader.SizeOfImage = align(virtual_offset + section_size, section_alignment);

   memcpy(pointer, section_data, section_size);

   return;
}