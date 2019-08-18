#include "acpi.h"
#include "string.h"

static acpi_rsdp_t *rsdp;

static acpi_xsdt_t *xsdt;
static acpi_rsdt_t *rsdt;

bool init_acpi(uintptr_t rsd_ptr)
{
    bool result = false;

    rsdp = (acpi_rsdp_t *)rsd_ptr;
    result = h_memcmp(rsdp->signature, "RSD PTR ", 8);
    if (!result)
    {
        goto finish;
    }

    xsdt = (acpi_xsdt_t *)rsdp->xsdt_address;
    result = h_memcmp(xsdt->header.signature, "XSDT", 4);
    if (!result)
    {
        goto finish;
    }

    rsdt = (acpi_rsdt_t *)(uintptr_t)rsdp->rsdt_address;
    result = h_memcmp(rsdt->header.signature, "RSDT", 4);
    if (!result)
    {
        // there is not RSDT in this system.
    }

    // The first entry must be FADT
    // acpi_dt_header_t *header = (acpi_dt_header_t *)xsdt->entries[0];
    acpi_fadt_t *fadt = (acpi_fadt_t *)xsdt->entries[0];
    result = h_memcmp(fadt->header.signature, "FACP", 4);
    if (!result)
    {
        goto finish;
    }

    result = true;

finish:
    return result;
}

bool get_mcfg_table(uintptr_t *ptr)
{
    bool result;
    uint64_t number_of_entries;

    // search in xsdt
    number_of_entries =
        (xsdt->header.length - sizeof(acpi_dt_header_t)) / sizeof(uint64_t);

    for (int i = 0; i < number_of_entries; ++i)
    {
        acpi_dt_header_t *header = (acpi_dt_header_t *)xsdt->entries[i];
        result = h_memcmp(header->signature, "MCFG", 4);
        if (result)
        {
            *ptr = (uintptr_t)xsdt->entries[i];
            goto find;
        }
    }

    // search in rsdt
    number_of_entries =
        (rsdt->header.length - sizeof(acpi_dt_header_t)) / sizeof(uint32_t);
    for (int i = 0; i < number_of_entries; ++i)
    {
        acpi_dt_header_t *header = (acpi_dt_header_t *)(uintptr_t)rsdt->entries[i];
        result = h_memcmp(header->signature, "MCFG", 4);
        if (result)
        {
            *ptr = (uintptr_t)rsdt->entries[i];
            goto find;
        }
    }

    return false;

find:
    return true;
}

bool get_pcie_configuration_addr(uintptr_t *addr)
{
    addr = NULL;
    bool result = false;

    uint64_t number_of_entries = 10;
    //(xsdt->header.length - sizeof(acpi_dt_header_t)) / sizeof(uint64_t *);

    for (int i = 1; i < number_of_entries; ++i)
    {
        acpi_dt_header_t *header = (acpi_dt_header_t *)xsdt->entries[i];
        result = h_memcmp(header->signature, "MCFG", 4);
        if (result)
        {
            // 44 is offset of the configuration space based address allocations
            // structure
            *addr = (uintptr_t)header + 44;
            goto finish;
        }
    }

finish:
    return result;
}
