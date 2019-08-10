#pragma once
#include <efi.h>
#include <efilib.h>

EFI_STATUS get_acpi_rsdp(EFI_SYSTEM_TABLE* system_table, EFI_PHYSICAL_ADDRESS* rsdp);

