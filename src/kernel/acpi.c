#include "acpi.h"
#include "string.h"
#include "utils.h"

#include "pci_device_struct.h"

static acpi_rsdp_t *rsdp;

static acpi_xsdt_t *xsdt;
static acpi_rsdt_t *rsdt;
static acpi_fadt_t *fadt;
static acpi_dsdt_t *dsdt;

static acpi_mcfg_t *mcfgt;

bool search_signature_in_xsdt(uintptr_t *ptr, const char *signature, int length)
{
    // search in xsdt
    int number_of_entries = (xsdt->header.length - sizeof(acpi_dt_header_t)) /
                            sizeof(xsdt->entries[0]);

    for (int i = 0; i < number_of_entries; ++i)
    {
        acpi_dt_header_t *header = (acpi_dt_header_t *)xsdt->entries[i];
        bool result = h_memcmp(header->signature, signature, length);
        if (result)
        {
            *ptr = (uintptr_t)xsdt->entries[i];
            goto find;
        }
    }

    *ptr = (uintptr_t)NULL;
    return false;
find:
    return true;
}

bool init_acpi(uintptr_t rsd_ptr)
{
    bool result = false;

    rsdp = (acpi_rsdp_t *)rsd_ptr;
    result = h_memcmp(rsdp->signature, "RSD PTR ", 8);
    if (!result)
    {
        goto fail;
    }

    xsdt = (acpi_xsdt_t *)rsdp->xsdt_address;
    result = h_memcmp(xsdt->header.signature, "XSDT", 4);
    if (!result)
    {
        goto fail;
    }

    //rsdt = (acpi_rsdt_t *)(uintptr_t)rsdp->rsdt_address;
    //result = h_memcmp(rsdt->header.signature, "RSDT", 4);
    //if (!result)
    //{}

    // acpi_dt_header_t *header = (acpi_dt_header_t *)xsdt->entries[0];
    //uintptr_t tmp_ptr;
    //result = search_signature_in_xsdt(&tmp_ptr, "FACP", 4);
    //if (!result)
    //{
    //    goto fail;
    //}
    //fadt = (acpi_fadt_t *)tmp_ptr;

    //dsdt = (acpi_dsdt_t *)fadt->x_dsdt;
    //result = h_memcmp(dsdt->header.signature, "DSDT", 4);
    //if (!result)
    //{
    //    goto fail;
    //}

    return true;

fail:
    return false;
}

bool setup_mcfg_table(void)
{
    bool result;
    uint64_t number_of_entries;

    result = search_signature_in_xsdt(&mcfgt, "MCFG", 4);
    if(result)
    {
        goto find;
    }

    // search in rsdt
    number_of_entries = (rsdt->header.length - sizeof(acpi_dt_header_t)) /
                        sizeof(rsdt->entries[0]);
    for (int i = 0; i < number_of_entries; ++i)
    {
        acpi_dt_header_t *header =
            (acpi_dt_header_t *)(uintptr_t)rsdt->entries[i];
        result = h_memcmp(header->signature, "MCFG", 4);
        if (result)
        {
            assert(false, "MCFG table found in RSDT. [NIY]");
        }
    }

    return false;

find:
    return true;
}

bool get_pci_config_space_addresses(void)
{
    bool result;
    if (mcfgt == NULL)
    {
        result = setup_mcfg_table();
        if (!result)
        {
            return false;
        }
    }
    assert(mcfgt != NULL, "something wrong...");

    int pci_device_num = (mcfgt->header.length - sizeof(acpi_dt_header_t)) /
                         sizeof(mcfgt->config_space[0]);

    for (int i = 0; i < pci_device_num; ++i)
    {
        pci_config_space_t *config_space =
            (pci_config_space_t *)mcfgt->config_space[i].base_address;

        printk((const char*)config_space->vendor_id);
        printk((const char*)config_space->device_id);
    }

    return true;
}
