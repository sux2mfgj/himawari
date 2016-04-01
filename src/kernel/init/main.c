#include <efi.h>
#include <efilib.h>

EFI_STATUS get_memory_map(EFI_MEMORY_DESCRIPTOR* memory_map, uintmax_t* memory_map_size, uintmax_t* map_key)
{

    EFI_STATUS Status = EFI_SUCCESS;
    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, memory_map_size, NULL,
                               NULL, NULL, NULL);
    if (memory_map_size == 0) {
        return Status;
    }

    Print(L"memmap size: %d\n", *memory_map_size);
    uintmax_t pages;
retry:

    *memory_map_size += (1 << 12);
    pages = (*memory_map_size / 0x1000);

    Status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData,
                              pages, memory_map);

    if(EFI_ERROR(Status)) {
        return Status;
    }

    uintmax_t descriptor_size, descriptor_version;

//typedef EFI_STATUS(EFIAPI * EFI_GET_MEMORY_MAP)
//  (   IN OUT UINTN *MemoryMapSize,
//      IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
//      OUT UINTN *MapKey,
//      OUT UINTN *DescriptorSize,
//      OUT UINT32 *DescriptorVersion)

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5, memory_map_size, memory_map, map_key, &descriptor_size, &descriptor_version);
    if(EFI_ERROR(Status)) {
        if(Status == EFI_BUFFER_TOO_SMALL) {
            

//typedef EFI_STATUS(EFIAPI * EFI_FREE_PAGES)
//  (   IN EFI_PHYSICAL_ADDRESS Memory,
//      IN UINTN Pages)
            Status = uefi_call_wrapper(BS->FreePages, 2, memory_map, pages);
            if(EFI_ERROR(Status)) {
                return Status;
            }
            goto retry; 
        }
        return Status;
    }

    Print(L"pages: %d\n", pages);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_DEVICE_PATH* Path;
    EFI_LOADED_IMAGE* LoadedImageParent;
    EFI_LOADED_IMAGE* LoadedImage;
    EFI_HANDLE Image;
    EFI_STATUS Status = EFI_SUCCESS;

    InitializeLib(ImageHandle, SystemTable);
    Print(L"Hello, EFI!\n");
    Print(L"boot loader 0x%08x\n", ImageHandle);

    EFI_MEMORY_DESCRIPTOR* memory_map;
    uintmax_t memory_map_size, map_key;

    Status = get_memory_map(memory_map, &memory_map_size, &map_key);
    if(EFI_ERROR(Status)) {
        Print(L"get_memory_map: %r\n", Status);
        while(1) {

        }
        return Status;
    }
    Print(L"memory_map: 0x%08x\n", memory_map);

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
    /*
    Status = uefi_call_wrapper(
        BS->OpenProtocol, 6, ImageHandle, &LoadedImageProtocol,
        &LoadedImageParent, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR(Status)) {
        Print(L"Could not get LoadedImageProtocol handler %r\n", Status);
        return Status;
    }

    Path = FileDevicePath(LoadedImageParent->DeviceHandle, L"\\kernel.efi");
    if (Path == NULL) {
        Print(L"Could not get device path.");
        return EFI_INVALID_PARAMETER;
    }


    Status = uefi_call_wrapper(BS->LoadImage, 6, FALSE, ImageHandle,
            Path, NULL, 
            0, &Image);

    if (EFI_ERROR(Status)) {
        Print(L"Could not load %r", Status);
        FreePool(Path);
        return Status;
    }

    Status = uefi_call_wrapper(BS->OpenProtocol, 6, Image, &LoadedImageProtocol,
                               &LoadedImage, ImageHandle, NULL,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status)) {
        Print(L"Could not get LoadedImageProtocol handler %r\n", Status);
        uefi_call_wrapper(BS->UnloadImage, 1, Image);
        FreePool(Path);
        return Status;
    }

    Status = uefi_call_wrapper(BS->StartImage, 3, Image, NULL, NULL);
    uefi_call_wrapper(BS->UnloadImage, 1, Image);
    FreePool(Path);
    */

    while(1){}
    return EFI_SUCCESS;
}
