#include <efi.h>
#include <efilib.h>

#include <stdint.h>
#include <stdbool.h>

EFI_STATUS get_memory_map(EFI_MEMORY_DESCRIPTOR* memory_map,
                          uintmax_t* memory_map_size,
                          uintmax_t* map_key)
{
    EFI_STATUS status = EFI_SUCCESS;
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, memory_map_size, NULL, NULL,
                               NULL, NULL);
    if (memory_map_size == 0) {
        return status;
    }

    Print(L"memmap size: %d\n", *memory_map_size);
    Print(L"DescriptorSize %d\n", sizeof(EFI_MEMORY_DESCRIPTOR));
    uintmax_t pages;
retry:

    *memory_map_size += (1 << 12);
    pages = (*memory_map_size / 0x1000);

    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
                               EfiLoaderData, pages, memory_map);
/*                                EfiLoaderData, pages, memory_map); */

    Print(L"memmap: 0x%08x\n", memory_map);
    if (EFI_ERROR(status)) {
        return status;
    }

    uintmax_t descriptor_size, descriptor_version;

    // typedef EFI_STATUS(EFIAPI * EFI_GET_MEMORY_MAP)
    //  (   IN OUT UINTN *MemoryMapSize,
    //      IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    //      OUT UINTN *MapKey,
    //      OUT UINTN *DescriptorSize,
    //      OUT UINT32 *DescriptorVersion)
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, memory_map_size, memory_map,
                               map_key, &descriptor_size, &descriptor_version);
    if (EFI_ERROR(status)) {
        if (status == EFI_BUFFER_TOO_SMALL) {
            // typedef EFI_STATUS(EFIAPI * EFI_FREE_PAGES)
            //  (   IN EFI_PHYSICAL_ADDRESS Memory,
            //      IN UINTN Pages)
            status = uefi_call_wrapper(BS->FreePages, 2, memory_map, pages);
            if (EFI_ERROR(status)) {
                return status;
            }
            goto retry;
        }
        return status;
    }

    Print(L"memmap size: %d\n", *memory_map_size);
    for (int i = 0; i < *memory_map_size; ++i) {
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
                break;
        }
    }

    Print(L"pages: %d\n", pages);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Wellcome to himawari!\n");

    EFI_STATUS status = EFI_SUCCESS;

    EFI_MEMORY_DESCRIPTOR* memory_map;
    uintmax_t memory_map_size, map_key;
    status = get_memory_map(memory_map, &memory_map_size, &map_key);

    if (EFI_ERROR(status)) {
        return status;
    }

    while (true) {
    }
    return EFI_SUCCESS;
}
