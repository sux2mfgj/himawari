#include <acpi.h>
#include <hm/print.h>
#include <hm/string.h>

static bool validate_rsdt(struct rsdt_t *rsdt) {
  if (!memcmp(rsdt->header.signature, DESC_TABLE_SIG_RSDT,
              sizeof(DESC_TABLE_SIG_RSDT) - 1))
    return false;

  return true;
}

static bool validate_xsdt(struct xsdt_t *xsdt) {

  if (!memcmp(xsdt->header.signature, DESC_TABLE_SIG_XSDT,
              sizeof(DESC_TABLE_SIG_XSDT) - 1))
    return false;

  return true;
}

int acpi_init(struct rsdp_v1_t *rsdp) {

  if (!memcmp(rsdp->signature, RSDP_SIGNATURE, sizeof(RSDP_SIGNATURE) - 1))
    return -1;

  void *root_sdt = NULL;
  switch (rsdp->revision) {
  case RSDP_REV_ACPI_1:
    kprintf("ACPI rev 1\n");
    root_sdt = (void *)(uint64_t)rsdp->rsdt_address;
    if (!validate_rsdt(root_sdt))
      return false;
    break;
  case RSDP_REV_ACPI_2: {
    kprintf("ACPI rev 2\n");
    struct rsdp_v2_t *rsdp_v2 = (struct rsdp_v2_t *)rsdp;
    root_sdt = (void *)rsdp_v2->xsdt_address;
    if (!validate_xsdt((struct xsdt_t *)root_sdt))
      return false;
    break;
  }
  default:
    kprintf("unknown revision type found\n");
    return -1;
  }

  return 0;
}
