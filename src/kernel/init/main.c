#include <efi.h>
#include <efilib.h>

#include <stdint.h>
#include <stdbool.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS status;
    uintmax_t memory_map_size;

    InitializeLib(ImageHandle, SystemTable);
    Print(L"efi_main: 0x%08x\n", efi_main);
    Print(L"0x%08x\n", ImageHandle);

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, NULL,
                               NULL, NULL, NULL);
    if (memory_map_size == 0) {
        return status;
    }

    EFI_MEMORY_DESCRIPTOR* memory_map;// = NULL;
    uintmax_t map_key, descriptor_size, descriptor_version;


    Print(L"memory_map_size: %d\n", memory_map_size);
    status = uefi_call_wrapper(BS->AllocatePool, 4, EfiLoaderData,
                               memory_map_size, (void **)memory_map);
/*     status = uefi_call_wrapper(BS->AllocatePool, 4, EfiBootServicesData, */
/*                                memory_map_size, memory_map); */
    if (EFI_ERROR(status)) {
        Print(L"allocate pool: %r\n", status);
        return status;
    }

    status =
        uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, (void *)memory_map,
                          &map_key, &descriptor_size, &descriptor_version);

    if (EFI_ERROR(status)) {
        Print(L"get memory map: %r\n", status);
        while (1) {
            
        }
        return status;
    }

    Print(L"memory_map addr 0x%08x\n", (uintptr_t)&memory_map[0]);
    Print(L"size %d\n", memory_map_size);

    uintptr_t max_start = 0;
    Print(L"aaa 0x%08x\n", max_start);
    for (int i = 0; i < memory_map_size; ++i) {

        switch (memory_map[i].Type) {
            case EfiLoaderCode:
                Print(L"1-");
            case EfiLoaderData:
                Print(L"2-");
            case EfiBootServicesCode:
                Print(L"3-");
            case EfiBootServicesData:
                Print(L"4-");
            case EfiConventionalMemory:
                if(max_start < memory_map[i].PhysicalStart) {
                    max_start = memory_map[i].PhysicalStart;
                }

                if (memory_map[i].Attribute & EFI_MEMORY_WB) {
                    Print(L"0x%08x, %d\n", memory_map[i].PhysicalStart,
                            memory_map[i].NumberOfPages);
                }
                else {
                    Print(L"\n");
                    break;
                }
/*             case EFI_ACPI_RECLAIM_MEMORY: */
/*                 break; */
/*             case EFI_ACPI_MEMORY_NVS: */
/*                 break; */
/*             case EFI_UNUSABLE_MEMORY: */
/*                 break; */
            default: 
                if(max_start < memory_map[i].PhysicalStart) {
                    max_start = memory_map[i].PhysicalStart;
                }
                break;
        }
    }
    Print(L"max_start 0x%08x\n", max_start);

/*     status = uefi_call_wrapper(BS->AllocatePool, 4, EfiBootServicesData, */
/*                                memory_map_size, memory_map); */
/*     if (EFI_ERROR(status)) { */
/*         Print(L"allocate pool: %r\n", status); */
/*         return status; */
/*     } */


    status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, map_key);
    if(EFI_ERROR(status)) {
        Print(L"exit: %r\n", status);
        while(1){}
        return status;
    }

    while (1){}
        
    return status;
}

