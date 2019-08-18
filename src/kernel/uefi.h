#pragma once

typedef unsigned short CHAR16;
typedef unsigned long long EFI_STATUS;
typedef void *EFI_HANDLE;

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (*EFI_TEXT_STRING) (struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *this,
        CHAR16 *string);

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void *a;
    EFI_TEXT_STRING OutputString;

} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct
{
    char a[52];
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
} EFI_SYSTEM_TABLE;


EFI_STATUS call_uefi_function(void *func, long number_of_args, ...);
