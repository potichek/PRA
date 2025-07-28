#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

PFLT_FILTER flt_handle = NULL;
const UNICODE_STRING protected_file = RTL_CONSTANT_STRING(L"virus.exe");

void get_finalcomponent(PUNICODE_STRING name, PUNICODE_STRING finalcomponent)
{
	int finalcomponent_offset;
	for (finalcomponent_offset = (name->Length) / sizeof(WCHAR);
		*((name->Buffer) + finalcomponent_offset) != '\\';
		finalcomponent_offset--);

	RtlInitUnicodeString(finalcomponent, (PCWSTR)((name->Buffer) + finalcomponent_offset + 1));
	return;
}

FLT_PREOP_CALLBACK_STATUS pre_operation
(
	PFLT_CALLBACK_DATA data,
	PCFLT_RELATED_OBJECTS flt_objects,
	PVOID* completion_context
)
{
	UNREFERENCED_PARAMETER(completion_context);

	if (flt_objects->FileObject->Flags & (FO_NAMED_PIPE | FO_MAILSLOT | FO_VOLUME_OPEN)) return FLT_PREOP_SUCCESS_NO_CALLBACK;

	const FLT_PARAMETERS parameters = data->Iopb->Parameters;
	if (
		FlagOn(parameters.Create.Options, FILE_DIRECTORY_FILE) ||
		FsRtlIsPagingFile(flt_objects->FileObject) ||
		FlagOn(parameters.Create.Options, FILE_OPEN_BY_FILE_ID))
	{
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	PFLT_FILE_NAME_INFORMATION filename_info = NULL;
	if (!NT_SUCCESS(FltGetFileNameInformation(data, FLT_FILE_NAME_NORMALIZED, &filename_info)))
	{
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	UNICODE_STRING finalcomponent;
	get_finalcomponent(&(filename_info->Name), &finalcomponent);

	if (RtlCompareUnicodeString(&finalcomponent, &protected_file, TRUE) != 0)
	{
		FltReleaseFileNameInformation(filename_info);
		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	data->IoStatus.Status = STATUS_ACCESS_DENIED;
	data->IoStatus.Information = IO_REPARSE;

	FltReleaseFileNameInformation(filename_info);
	return FLT_PREOP_COMPLETE;
}

const FLT_OPERATION_REGISTRATION callbacks[] =
{
	{IRP_MJ_CREATE, 0, pre_operation, NULL, NULL},
	{IRP_MJ_OPERATION_END}
};

NTSTATUS flt_unload(FLT_FILTER_UNLOAD_FLAGS flags)
{
	UNREFERENCED_PARAMETER(flags);

	DbgPrint("Unload\n");
	FltUnregisterFilter(flt_handle);

	return STATUS_SUCCESS;
}

const FLT_REGISTRATION flt_registration =
{
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	callbacks,
	flt_unload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = FltRegisterFilter(DriverObject, &flt_registration, &flt_handle);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Cant register filter: %X\n", status);
		return status;
	}

	status = FltStartFiltering(flt_handle);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Cant start filter: %X\n", status);
		FltUnregisterFilter(flt_handle);
		return status;
	}

	DbgPrint("Succesful setup: %X\n", status);
	return status;
}