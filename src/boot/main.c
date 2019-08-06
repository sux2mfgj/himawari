#include <efi.h>
#include <efilib.h>

CHAR16* convery_type_into_string(EFI_MEMORY_TYPE type)
{
    switch (type) {
        case EfiReservedMemoryType:
            return L"EfiReserved            ";
        case EfiLoaderCode:
            return L"EfiLoaderCode          ";
        case EfiLoaderData:
            return L"EfiLoaderData          ";
        case EfiBootServicesCode:
            return L"EfiBootServicesCode    ";
        case EfiBootServicesData:
            return L"EfiBootServicesData    ";
        case EfiRuntimeServicesCode:
            return L"EfiRuntimeServicesCode ";
        case EfiRuntimeServicesData:
            return L"EfiRuntimeServicesData ";
        case EfiConventionalMemory:
            return L"EfiConventionalMemory  ";
        case EfiUnusableMemory:
            return L"EfiUnusableMemory      ";
        case EfiACPIReclaimMemory:
            return L"EfiACPIReclaimMemory   ";
        case EfiACPIMemoryNVS:
            return L"EfiACPIMemoryNVS       ";
        case EfiMemoryMappedIO:
            return L"EfiMemoryMappedIO      ";
        case EfiMemoryMappedIOPortSpace:
            return L"EfiMMIOPortSpace       ";
        case EfiPalCode:
            return L"EfiPalCode             ";
        case EfiMaxMemoryType:
            return L"EfiMaxMemoryType       ";
        default:
            return L"Unknown                ";
    }
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    EFI_STATUS status = EFI_SUCCESS;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simpleFile;
    status = uefi_call_wrapper(BS->LocateProtocol, 3,
                               &gEfiSimpleFileSystemProtocolGuid, NULL,
                               (void**)&simpleFile);

    if (EFI_ERROR(status)) {
        Print(L"1. %r", status);
        return status;
    }

    EFI_FILE_PROTOCOL* Root = NULL;
    status = uefi_call_wrapper(simpleFile->OpenVolume, 2, simpleFile, &Root);
    if (EFI_ERROR(status)) {
        Print(L"2. %r", status);
        return status;
    }

    CHAR16* filename = L"\\EFI\\BOOT\\kernel.elf";
    EFI_FILE_PROTOCOL* File;
    status = uefi_call_wrapper(Root->Open, 5, Root, &File, filename,
                               EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print(L"3. %r", status);
        return status;
    }

    UINTN file_info_buffer_size = sizeof(EFI_FILE_INFO) * 10;
    UINTN file_info_buffer[file_info_buffer_size];
    status = uefi_call_wrapper(File->GetInfo, 4, File, &gEfiFileInfoGuid, &file_info_buffer_size, file_info_buffer);
    if(EFI_ERROR(status))
    {
        Print(L"File->GetInfo %r\n", status);
        return status;
    }
    UINTN file_size = ((EFI_FILE_INFO*)file_info_buffer)->FileSize;
    // 4k aligned
    UINTN enough_page_size = (file_size + 0x1000 - 1) / 0x1000;

    EFI_PHYSICAL_ADDRESS kernel_load_page_head = 0x100000;
    Print(L"elf file size %d, page num %d\n", file_size, enough_page_size);

    // allocate relocate destination memory that is started at 0x100000.
    // this address is configurated by the linker script.
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress, EfiLoaderData, enough_page_size, &kernel_load_page_head);
    if(EFI_ERROR(status))
    {
        Print(L"AllocatePages %r\n", status);
        return status;
    }

    EFI_PHYSICAL_ADDRESS tmp_page_address = 0;
    // and then, allocate memory to load kernel.elf temporary.
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, enough_page_size, &tmp_page_address);
    if(EFI_ERROR(status))
    {
        Print(L"AllocatePages %r\n", status);
        return status;
    }
    Print(L"temporary page address 0x08%x\n", tmp_page_address);

    status = uefi_call_wrapper(File->Read, 3, File, &enough_page_size, tmp_page_address);
    if(EFI_ERROR(status))
    {
        Print(L"Read %r\n", status);
        return status;
    }

    // memory map
    VOID* memory_map_buffer;
    UINTN buffer_size = 0x1000;
    UINTN map_size;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;

    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, buffer_size,
                               &memory_map_buffer);
    if (EFI_ERROR(status)) {
        Print(L"4. %r", status);
        return status;
    }

    status =
        uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, memory_map_buffer,
                          &map_key, &descriptor_size, &descriptor_version);
    if (EFI_ERROR(status)) {
        Print(L"5. %r", status);
        return status;
    }

    UINTN descriptor_count = map_size / descriptor_size;
    EFI_MEMORY_DESCRIPTOR* memory_descriptor_ptr;
    for (int i = 0; i < descriptor_count; ++i) {
        memory_descriptor_ptr =
            (EFI_MEMORY_DESCRIPTOR*)(memory_map_buffer + (i * descriptor_size));
        Print(L"type: %s, phy start 0x%08x, pages %d\n",
                convery_type_into_string(memory_descriptor_ptr->Type),
                memory_descriptor_ptr->PhysicalStart,
                memory_descriptor_ptr->NumberOfPages
                );
    }

    FreePool(memory_map_buffer);

    while (1) {
    }
}
