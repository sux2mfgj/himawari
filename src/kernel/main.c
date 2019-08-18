#include <stdbool.h>

#include "boot_arg.h"

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

    uintptr_t mcfg_table;
    result = get_mcfg_table((uintptr_t *)&mcfg_table);
    if (!result)
    {
        // print("PCIe device is not found");
        //goto fail;
    }

    result = init_pci(bootinfo->is_support_pci);
    if(!result)
    {
        //printk("there is not pci device in this system");
        //goto fail
    }

    EFI_SYSTEM_TABLE *system_table =
        (EFI_SYSTEM_TABLE *)bootinfo->uefi_system_table;

    EFI_STATUS status;
    status = call_uefi_function(system_table->ConOut->OutputString, 2,
                                system_table->ConOut, L"Hello in kernel\n");

fail:
    while (true)
    {
        __asm__ volatile("hlt");
    }
}
