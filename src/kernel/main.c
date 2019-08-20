#include <stdbool.h>

#include "boot_arg.h"

#include "utils.h"
#include "memory_manager.h"

#include "acpi.h"
#include "uefi.h"
#include "pci.h"

void start_kernel(struct boot_argument *bootinfo)
{
    bool result = false;

    result = init_memory_manager(bootinfo->number_of_meminfo, bootinfo->meminfo);
    if(!result)
    {
        goto fail;
    }

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

fail:
    assert(false, "reach the start_kernel end");
}
