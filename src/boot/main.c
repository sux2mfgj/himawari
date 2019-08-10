#include <assert.h>

#include "boot.h"

#include "boot_arg.h"
#include "elf.h"

CHAR16 *convery_type_into_string(EFI_MEMORY_TYPE type)
{
    switch (type)
    {
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

EFI_STATUS relocate_kernel_elf(EFI_PHYSICAL_ADDRESS temporary_loaded_address,
                               EFI_PHYSICAL_ADDRESS *kernel_memory_start,
                               UINTN *number_of_pages,
                               EFI_PHYSICAL_ADDRESS *kernel_entry)

{
    EFI_STATUS status = EFI_SUCCESS;

    Elf64_Ehdr *elf_header = (Elf64_Ehdr *)temporary_loaded_address;
    Print(L"%x %x %x %x\n", elf_header->e_ident[EI_MAG0],
          elf_header->e_ident[EI_MAG1], elf_header->e_ident[EI_MAG2],
          elf_header->e_ident[EI_MAG3]);

    Print(L"file class  %x\n", elf_header->e_ident[4]);
    Print(L"data encode %x\n", elf_header->e_ident[5]);
    Print(L"version     %x\n", elf_header->e_ident[6]);
    Print(L"OS ABI      %x\n", elf_header->e_ident[7]);
    Print(L"entry addr  %x\n", elf_header->e_entry);

    Print(L"p header %d\n", elf_header->e_phnum);
    Print(L"p header offset %d\n", elf_header->e_phoff);

    Elf64_Phdr *elf_program_header =
        (Elf64_Phdr *)(temporary_loaded_address + elf_header->e_phoff);

    EFI_PHYSICAL_ADDRESS lowest_address = ~0ULL;
    EFI_PHYSICAL_ADDRESS highest_address = 0;

    for (int i = 0; i < elf_header->e_phnum; ++i)
    {
        if (elf_program_header[i].p_type == PT_LOAD)
        {
            EFI_PHYSICAL_ADDRESS physical_addr = elf_program_header[i].p_paddr;
            UINTN segment_size = elf_program_header[i].p_memsz;

            if (lowest_address > physical_addr)
            {
                lowest_address = physical_addr;
            }
            if (highest_address < (physical_addr + segment_size))
            {
                highest_address = physical_addr + segment_size;
            }

            Print(L"phys addr 0x%08x, size 0x%x\n", physical_addr,
                  segment_size);
        }
    }

    assert(lowest_address < highest_address);
    Print(L"allocate range 0x%08x - 0x%08x\n", lowest_address, highest_address);

    EFI_PHYSICAL_ADDRESS aligned_lowest = lowest_address & ~0xfff;
    EFI_PHYSICAL_ADDRESS aligned_highest =
        (highest_address + 0x1000 - 1) & ~0xfff;

    UINTN alloc_page_num = (aligned_highest - aligned_lowest) / 0x1000;

    Print(L"aligned addressed 0x%08x 0x%08x [%d page]\n", aligned_lowest,
          aligned_highest, alloc_page_num);

    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAddress,
                               EfiLoaderData, alloc_page_num, &aligned_lowest);
    if (EFI_ERROR(status))
    {
        Print(L"AllocatePages %r\n", status);
        return status;
    }

    for (int i = 0; i < elf_header->e_phnum; ++i)
    {
        if (elf_program_header[i].p_type == PT_LOAD)
        {
            UINTN offset = elf_program_header[i].p_offset;
            UINTN size = elf_program_header[i].p_filesz;
            CopyMem((void *)aligned_lowest,
                    (void *)(temporary_loaded_address + offset), size);
        }
    }

    *kernel_memory_start = aligned_lowest;
    *number_of_pages = alloc_page_num;
    *kernel_entry = elf_header->e_entry;

    return status;
}

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);
    EFI_STATUS status = EFI_SUCCESS;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simpleFile;
    status = uefi_call_wrapper(BS->LocateProtocol, 3,
                               &gEfiSimpleFileSystemProtocolGuid, NULL,
                               (void **)&simpleFile);
    if (EFI_ERROR(status))
    {
        Print(L"1. %r", status);
        return status;
    }

    EFI_FILE_PROTOCOL *Root = NULL;
    status = uefi_call_wrapper(simpleFile->OpenVolume, 2, simpleFile, &Root);
    if (EFI_ERROR(status))
    {
        Print(L"2. %r", status);
        return status;
    }

    CHAR16 *filename = L"\\EFI\\BOOT\\kernel.elf";
    EFI_FILE_PROTOCOL *File;
    status = uefi_call_wrapper(Root->Open, 5, Root, &File, filename,
                               EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status))
    {
        Print(L"3. %r", status);
        return status;
    }

    UINTN file_info_buffer_size = sizeof(EFI_FILE_INFO) * 10;
    UINTN file_info_buffer[file_info_buffer_size];
    status = uefi_call_wrapper(File->GetInfo, 4, File, &gEfiFileInfoGuid,
                               &file_info_buffer_size, file_info_buffer);
    if (EFI_ERROR(status))
    {
        Print(L"File->GetInfo %r\n", status);
        return status;
    }
    UINTN file_size = ((EFI_FILE_INFO *)file_info_buffer)->FileSize;
    // 4k aligned
    UINTN enough_page_size = (file_size + 0x1000 - 1) / 0x1000;

    Print(L"elf file size %d, page num %d\n", file_size, enough_page_size);

    EFI_PHYSICAL_ADDRESS tmp_page_address = 0;
    // and then, allocate memory to load kernel.elf temporary.
    status =
        uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData,
                          enough_page_size, &tmp_page_address);
    if (EFI_ERROR(status))
    {
        Print(L"AllocatePages %r\n", status);
        return status;
    }
    Print(L"temporary page address 0x08%x\n", tmp_page_address);

    status = uefi_call_wrapper(File->Read, 3, File, &file_size,
                               (void *)tmp_page_address);
    if (EFI_ERROR(status))
    {
        Print(L"Read %r\n", status);
        return status;
    }

    EFI_PHYSICAL_ADDRESS kernel_memory_start;
    UINTN kernel_memory_page_number;
    EFI_PHYSICAL_ADDRESS kernel_entry;
    status = relocate_kernel_elf(tmp_page_address, &kernel_memory_start,
                                 &kernel_memory_page_number, &kernel_entry);
    if (EFI_ERROR(status))
    {
        Print(L"relocation %r\n", status);
        return status;
    }
    FreePool((void *)tmp_page_address);

    EFI_PHYSICAL_ADDRESS boot_info_addr;

    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
                               EfiLoaderData, 1, &boot_info_addr);
    if (EFI_ERROR(status))
    {
        Print(L"AllocatePages %r\n", status);
        return status;
    }

    struct boot_argument *boot_arg = (struct boot_argument *)boot_info_addr;
    boot_arg->number_of_meminfo = 0;
    Print(L"boot_arg is here 0x%08x\n", (EFI_PHYSICAL_ADDRESS)boot_arg);

    EFI_PHYSICAL_ADDRESS kernel_stack_end;
    UINTN kernel_stack_pages = 1;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages,
                               EfiLoaderData, kernel_stack_pages, &kernel_stack_end);
    if (EFI_ERROR(status))
    {
        Print(L"AllocatePages %r\n", status);
        return status;
    }
    boot_arg->kernel_stack_address = kernel_stack_end - (0x1000 * kernel_stack_pages);

    // for ACPI
    EFI_PHYSICAL_ADDRESS acpi_rsdp;
    status = get_acpi_rsdp(SystemTable, &acpi_rsdp);
    if(EFI_ERROR(status))
    {
        Print(L"acpi %r\n", status);
        return status;
    }
    Print(L"ACPI RSDP 0x%08x\n", acpi_rsdp);
    boot_arg->acpi_rsdp = acpi_rsdp;

    // to get the memory map
    VOID *memory_map_buffer;
    UINTN buffer_size = 0x1000;
    UINTN map_size = 0x1000;
    UINTN map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;

    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, buffer_size,
                               &memory_map_buffer);
    if (EFI_ERROR(status))
    {
        Print(L"4. %r", status);
        return status;
    }

    status =
        uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, memory_map_buffer,
                          &map_key, &descriptor_size, &descriptor_version);
    if (EFI_ERROR(status))
    {
        Print(L"5. %r\n", status);
        return status;
    }

    UINTN descriptor_count = map_size / descriptor_size;
    EFI_MEMORY_DESCRIPTOR *memory_descriptor_ptr;
    for (int i = 0; i < descriptor_count; ++i)
    {
        memory_descriptor_ptr =
            (EFI_MEMORY_DESCRIPTOR *)(memory_map_buffer +
                                      (i * descriptor_size));
        Print(L"type: %s, phy start 0x%08x, pages %d\n",
              convery_type_into_string(memory_descriptor_ptr->Type),
              memory_descriptor_ptr->PhysicalStart,
              memory_descriptor_ptr->NumberOfPages);

        if (memory_descriptor_ptr->Type == EfiConventionalMemory)
        {
            struct memory_info *info =
                &boot_arg->meminfo[boot_arg->number_of_meminfo];
            info->base_address =
                (uintptr_t)memory_descriptor_ptr->PhysicalStart;
            info->number_of_pages =
                (size_t)memory_descriptor_ptr->NumberOfPages;
            info->type = PAGE_TYPE_FREE;
            boot_arg->number_of_meminfo++;
        }
        //TODO add another memory regions info
    }

    do
    {
        map_size = 0x1000;
        status =
            uefi_call_wrapper(BS->GetMemoryMap, 5, &map_size, memory_map_buffer,
                              &map_key, &descriptor_size, &descriptor_version);
        if (EFI_ERROR(status))
        {
            Print(L"5. %r\n", status);
            return status;
        }
        status =
            uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, map_key);
    } while (EFI_ERROR(status));

    __asm__ volatile("movq %0, %%rdi":  "=m"(boot_arg) :: "rdi");
    __asm__ volatile("jmpq *%0" : : "m"(kernel_entry));

    while (1)
    {
    }
}
