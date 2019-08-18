#include <stdbool.h>

#include "boot_arg.h"

#include "utils.h"

#include "acpi.h"
#include "uefi.h"
#include "pci.h"

void start_kernel(struct boot_argument *bootinfo)
{
    bool result = false;

    result = init_acpi(bootinfo->acpi_rsdp);
    if (!result)
    {
        goto fail;
    }

    result = get_pci_config_space_addresses();
    if(!result)
    {
        goto fail;
    }

    result = init_pci(bootinfo->is_support_pci);
    if(!result)
    {
        //printk("there is not pci device in this system");
        goto fail;
    }

    EFI_SYSTEM_TABLE *system_table =
        (EFI_SYSTEM_TABLE *)bootinfo->uefi_system_table;

    EFI_STATUS status;
    status = call_uefi_function(system_table->ConOut->OutputString, 2,
                                system_table->ConOut, L"Hello in kernel\n");

fail:
    assert(false, "reach the start_kernel end");
}
