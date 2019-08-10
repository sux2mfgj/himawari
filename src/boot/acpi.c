#include "boot.h"
#include "string.h"

#include <stdbool.h>

#define EFI_ACPI_TABLE_GUID \
    {0x8868e871,0xe4f1,0x11d3, {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}

#define EFI_GUID_DATA4_SIZE (8)

bool compare_efi_guid(EFI_GUID* left, EFI_GUID* right)
{
    if(left->Data1 != right->Data1
            || left->Data2 != right->Data2
            || left->Data3 != right->Data3)
    {
        return false;
    }

    return h_memcmp(left->Data4, right->Data4, EFI_GUID_DATA4_SIZE);
}

EFI_STATUS get_acpi_rsdp(EFI_SYSTEM_TABLE* system_table, EFI_PHYSICAL_ADDRESS* rsdp)
{
    EFI_STATUS status = EFI_INVALID_PARAMETER;

    EFI_GUID acpi_guid = EFI_ACPI_TABLE_GUID;

    for(int i=0; i<system_table->NumberOfTableEntries; ++i)
    {
        EFI_CONFIGURATION_TABLE* table = system_table->ConfigurationTable + i;
        if(compare_efi_guid(&table->VendorGuid, &acpi_guid)) {

            *rsdp = (EFI_PHYSICAL_ADDRESS)table->VendorTable;
            status = EFI_SUCCESS;
            goto finish;
        }
    }

finish:
    return status;
}
