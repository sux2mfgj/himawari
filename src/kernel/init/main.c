#include <efi.h>
#include <efilib.h>

/* #include <EfiTypes.h> */

#include <stdint.h>
#include <stdbool.h>

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS status;
    uintmax_t memory_map_size;

    InitializeLib(ImageHandle, SystemTable);
    Print(L"0x%08x\n", efi_main);

    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, NULL,
                               NULL, NULL, NULL);
    if (memory_map_size == 0) {
        return status;
    }

    EFI_MEMORY_DESCRIPTOR* memory_map;// = NULL;
    uintmax_t map_key, descriptor_size, descriptor_version;

    status = uefi_call_wrapper(BS->AllocatePool, 4, EfiBootServicesData,
                               memory_map_size, memory_map);
    if (EFI_ERROR(status)) {
        return status;
    }

    status =
        uefi_call_wrapper(BS->GetMemoryMap, 5, &memory_map_size, memory_map,
                          &map_key, &descriptor_size, &descriptor_version);

    if (EFI_ERROR(status)) {
        return status;
    }

    Print(L"0x%08x\n", memory_map);
    Print(L"%d\n", memory_map_size);

    for (int i = 0; i < memory_map_size; ++i) {

        switch (memory_map[i].Type) {
            case EfiLoaderCode:
            case EfiLoaderData:
            case EfiBootServicesCode:
            case EfiBootServicesData:
            case EfiConventionalMemory:
                if (memory_map[i].Attribute & EFI_MEMORY_WB) {
                    Print(L"0x%08x, %d\n", memory_map[i].PhysicalStart,
                            memory_map[i].NumberOfPages);
                }
                else {
                    break;
                }
/*             case EFI_ACPI_RECLAIM_MEMORY: */
/*                 break; */
/*             case EFI_ACPI_MEMORY_NVS: */
/*                 break; */
/*             case EFI_UNUSABLE_MEMORY: */
/*                 break; */
            default: 
                break;
        }

    }

    while (1)
        ;
}
