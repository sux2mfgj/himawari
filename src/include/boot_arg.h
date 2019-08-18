#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


enum page_type
{
    PAGE_TYPE_FREE,
    PAGE_TYPE_KERNEL,
    // TODO
};

struct memory_info
{
    uintptr_t base_address;
    size_t number_of_pages;
    enum page_type type;
};

struct boot_argument
{
    uintptr_t kernel_stack_address;
    uintptr_t acpi_rsdp;
    uintptr_t uefi_system_table;
    bool is_support_pci;
    size_t number_of_meminfo;
    struct memory_info meminfo[];
};
