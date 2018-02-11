/*
struct EFI_SYSTEM_TABLE {
        char _buf[60];
        struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
                void *_buf;
                unsigned long long (*OutputString)(struct
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, unsigned short *); } *ConOut;
};

void efi_main(void *ImageHandle __attribute__ ((unused)), struct
EFI_SYSTEM_TABLE *SystemTable)
{
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello
UEFI!\n"); while (1);
}
*/
#include <stdbool.h>
#include <stddef.h>

#include <efi.h>
#include <efilib.h>

EFI_SYSTEM_TABLE* efi_system_table;

void* uefi_malloc(size_t n)
{
    void* result_addr = NULL;
    EFI_STATUS status = EFI_SUCCESS;
    status = uefi_call_wrapper(efi_system_table->BootServices->AllocatePool, 3,
                               EfiLoaderData, n, &result_addr);
    if (EFI_ERROR(status)) {
        return NULL;
    }
    Print(L"0x%x\n", result_addr);
    return result_addr;
}

EFI_STATUS uefi_mfree(void* addr)
{
    EFI_STATUS status =
        uefi_call_wrapper(efi_system_table->BootServices->FreePool, 1, addr);
    return status;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    efi_system_table = SystemTable;
    Print(L"Hello, world!\n");

    EFI_STATUS status = EFI_SUCCESS;
    ;
    UINTN mapSize = 0, mapKey, descriptorSize;
    EFI_MEMORY_DESCRIPTOR* memoryMap = NULL;
    status =
        uefi_call_wrapper((void*)SystemTable->BootServices->GetMemoryMap, 5,
                          &mapSize, memoryMap, &mapKey, &descriptorSize, NULL);
    Print(L"0x%x 0x%x 0x%x\n", mapSize + descriptorSize, mapKey,
          descriptorSize);
retry:
    if (memoryMap != NULL) {
        status = uefi_mfree(memoryMap);
    }
    if (status == EFI_BUFFER_TOO_SMALL) {
        Print(L"buffer too small\n");
        memoryMap = uefi_malloc(mapSize + descriptorSize);
        if (memoryMap == NULL) {
            Print(L"uefi_malloc failed\n");
            goto error;
        }
        status = uefi_call_wrapper(
            (void*)SystemTable->BootServices->GetMemoryMap, 5, &mapSize,
            memoryMap, &mapKey, &descriptorSize, NULL);
        Print(L"0x%x 0x%x 0x%x 0x%x\n", mapSize, mapKey, descriptorSize,
              sizeof(EFI_MEMORY_DESCRIPTOR));
    }

    if (status != EFI_SUCCESS) {
        switch (status) {
            case EFI_BUFFER_TOO_SMALL:
                Print(L"buffer too small\n");
                goto retry;
            case EFI_INVALID_PARAMETER:
                Print(L"invalid parameter\n");
                goto error;
            default:
                Print(L"what's happen?\n");
                goto error;
        }
    }
    else {
        Print(L"GetMemoryMap succeed\n");
    }

//     EFI_MEMORY_DESCRIPTOR* desc = memoryMap;
    for (int i = 0; i < (mapSize / descriptorSize); ++i) {
//         Print(L"0x%x 0x%x\n", memoryMap[i].PhysicalStart, memoryMap[i].NumberOfPages);
//         switch (desc->Type) {
        switch (memoryMap[i].Type) {
            case EfiReservedMemoryType:
                Print(L"reserved\n");
                break;
            case EfiLoaderCode:
                break;
            case EfiLoaderData:
                break;
            case EfiBootServicesCode:
                break;
            case EfiBootServicesData:
                break;
            case EfiRuntimeServicesCode:
                break;
            case EfiRuntimeServicesData:
                break;
            case EfiConventionalMemory:
                Print(L"conventional\n");
                break;
            default:
//                 Print(L"def %d\n", desc->Type);
                break;
        }
//        desc = (void*)desc + descriptorSize;
    }

    while (true) {
    };
    return status;
error:
    while (true) {
    };
}
