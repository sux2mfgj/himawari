#include <efi.h>
#include <efilib.h>

#include <kernel.h>
#include <init.h>

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
    uintmax_t pages;
retry:

    *memory_map_size += (1 << 12);
    pages = (*memory_map_size / 0x1000);

    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
                               EfiLoaderData, pages, memory_map);

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

    /*     status = uefi_call_wrapper(BS->ExitBootServices, 2,
     * ImageHandle,mapKey); */
    /*     if(EFI_ERROR(status)) { */
    /*         return status; */
    /*     } */

    Print(L"pages: %d\n", pages);

    return EFI_SUCCESS;
}

void putc(const char c)
{
    unsigned short* vram_TextMode;
    vram_TextMode = (unsigned short*)0x000B8000;

    *vram_TextMode = (0x07 << 8) | c;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    {
        EFI_DEVICE_PATH* Path;
        EFI_LOADED_IMAGE* LoadedImageParent;
        EFI_LOADED_IMAGE* LoadedImage;
        EFI_HANDLE Image;
        EFI_STATUS status = EFI_SUCCESS;

        InitializeLib(ImageHandle, SystemTable);
        Print(L"Hello, EFI!\n");
        Print(L"boot loader 0x%08x\n", ImageHandle);

        EFI_MEMORY_DESCRIPTOR* memory_map;
        uintmax_t memory_map_size, map_key;

        status = get_memory_map(memory_map, &memory_map_size, &map_key);
        if (EFI_ERROR(status)) {
            Print(L"get_memory_map: %r\n", status);
            while (1) {
            }
            return status;
        }

        status =
            uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, map_key);
        if (EFI_ERROR(status)) {
            return status;
        }

        for (int i = 0; i < memory_map_size; ++i) {
            switch (memory_map[i].Type) {
                case EfiLoaderCode:
                case EfiLoaderData:
                case EfiBootServicesCode:
                case EfiBootServicesData:
                case EfiConventionalMemory:
                    if (memory_map[i].Attribute & EFI_MEMORY_WB) {
                        putc('a');
                        /*                             Print(L"0x%08x, %d\n", */
                        /*                                     memory_map[i].PhysicalStart,
                         */
                        /*                                     memory_map[i].NumberOfPages);
                         */
                    }
                    else {
                        break;
                    }
                    break;

                //                        case EFI_ACPI_RECLAIM_MEMORY:
                //                            break;
                //                        case EFI_ACPI_MEMORY_NVS:
                //                            break;
                //                        case EFI_UNUSABLE_MEMORY:
                //                            break;
                default:
                    break;
            }
        }

        /*
        status = uefi_call_wrapper(
            BS->OpenProtocol, 6, ImageHandle, &LoadedImageProtocol,
            &LoadedImageParent, ImageHandle, NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if (EFI_ERROR(status)) {
            Print(L"Could not get LoadedImageProtocol handler %r\n", status);
            return status;
        }

        Path = FileDevicePath(LoadedImageParent->DeviceHandle, L"\\kernel.efi");
        if (Path == NULL) {
            Print(L"Could not get device path.");
            return EFI_INVALID_PARAMETER;
        }


        status = uefi_call_wrapper(BS->LoadImage, 6, FALSE, ImageHandle,
                Path, NULL,
                0, &Image);

        if (EFI_ERROR(status)) {
            Print(L"Could not load %r", status);
            FreePool(Path);
            return status;
        }

        status = uefi_call_wrapper(BS->OpenProtocol, 6, Image,
        &LoadedImageProtocol,
                                   &LoadedImage, ImageHandle, NULL,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (EFI_ERROR(status)) {
            Print(L"Could not get LoadedImageProtocol handler %r\n", status);
            uefi_call_wrapper(BS->UnloadImage, 1, Image);
            FreePool(Path);
            return status;
        }

        status = uefi_call_wrapper(BS->StartImage, 3, Image, NULL, NULL);
        uefi_call_wrapper(BS->UnloadImage, 1, Image);
        FreePool(Path);
        */
    }

    status_code code;
    code = setup_arch();
    if (code) {
    }
    else {
        while (1) {
        }
    }

    while (1) {
    }
    return EFI_SUCCESS;
}
