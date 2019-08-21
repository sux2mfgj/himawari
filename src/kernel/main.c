#include <stdbool.h>

#include "boot_arg.h"

#include "memory_manager.h"
#include "utils.h"

#include "acpi.h"
#include "apic.h"
#include "pci.h"
#include "uefi.h"

void start_kernel(struct boot_argument *bootinfo)
{
    bool result = false;

    result =
        init_memory_manager(bootinfo->number_of_meminfo, bootinfo->meminfo);
    if (!result)
    {
        goto fail;
    }

    result = init_apic();
    if (!result)
    {
        goto fail;
    }

    result = init_acpi(bootinfo->acpi_rsdp);
    if (!result)
    {
        goto fail;
    }

    result = init_pci(bootinfo->is_support_pci);
    if (!result)
    {
        // printk("there is not pci device in this system");
        goto fail;
    }

fail:
    assert(false, "reach the start_kernel end");
}
