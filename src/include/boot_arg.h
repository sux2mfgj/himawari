#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "memory_info.h"

struct boot_argument
{
    uintptr_t kernel_stack_address;
    uintptr_t acpi_rsdp;
    uintptr_t uefi_system_table;
    bool is_support_pci;
    size_t number_of_meminfo;
    struct memory_info meminfo[];
};
