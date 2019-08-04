#include <efi.h>
#include <efilib.h>

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

    Print(L"pages: %d\n", pages);

    return EFI_SUCCESS;
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

        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simpleFile;
        EFI_FILE_PROTOCOL* root;
        EFI_FILE_PROTOCOL* file;
        EFI_GUID guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
        EFI_HANDLE* handles = NULL;
        uintmax_t handleCount = 0;

        status = uefi_call_wrapper(BS->LocateProtocol, 3, &guid, NULL,
                                   (void**)&simpleFile);
        if (EFI_ERROR(status)) {
            Print(L"%r", status);
            while (1) {
            }
        }

        status =
            uefi_call_wrapper(simpleFile->OpenVolume, 2, simpleFile, &root);
        if (EFI_ERROR(status)) {
            Print(L"%r", status);
            while (1) {
            }
        }

        // If you initialize memory_map by NULL, GetMemoryMap function return
        // INVALID_PARAMETER.
        EFI_MEMORY_DESCRIPTOR* memory_map;
        uintmax_t memory_map_size, map_key;

        status = get_memory_map(memory_map, &memory_map_size, &map_key);
        if (EFI_ERROR(status)) {
            Print(L"get_memory_map: %r\n", status);
            while (1) {
            }
            return status;
        }

        /*         status = */
        /*             uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle,
         * map_key); */
        /*         if (EFI_ERROR(status)) { */
        /*             return status; */
        /*         } */

        /*
        uintptr_t max_addr = 0x0;
        for (int i = 0; i < memory_map_size; ++i) {
            switch (memory_map[i].Type) {
                case EfiLoaderCode:
                    Print(L"1");
                case EfiLoaderData:
                    Print(L"1");
                case EfiBootServicesCode:
                    Print(L"1");
                case EfiBootServicesData:
                    Print(L"1");
                case EfiConventionalMemory:
                    if (memory_map[i].Attribute & EFI_MEMORY_WB) {
                        Print(L"1");
                        Print(L"0x%08x, %d\n",
                                memory_map[i].PhysicalStart,
                                memory_map[i].NumberOfPages);
                        if(max_addr < memory_map[i].PhysicalStart) {
                            Print(L"%d, %d\n", max_addr,
        memory_map[i].PhysicalStart); Print(L"1 max_addr 0x%08x, 0x%08x\n",
        max_addr, memory_map[i].PhysicalStart); max_addr =
        memory_map[i].PhysicalStart;
                        }

                    }
                    else {
        //                break;
                    }
                    break;

                //                        case EFI_ACPI_RECLAIM_MEMORY:
                //                            break;
                //                        case EFI_ACPI_MEMORY_NVS:
                //                            break;
                //                        case EFI_UNUSABLE_MEMORY:
                //                            break;
                default:
                    if(max_addr < memory_map[i].PhysicalStart) {
                        Print(L"%d < %d: %d\n",
                                max_addr,
                                memory_map[i].PhysicalStart,
                                (max_addr < memory_map[i].PhysicalStart));

                        Print(L"1 max_addr 0x%08x, 0x%08x\n", max_addr,
        memory_map[i].PhysicalStart); max_addr = memory_map[i].PhysicalStart;
                    }
                    break;
            }
        }
*/

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

    while (1) {
    }
    return EFI_SUCCESS;
}
