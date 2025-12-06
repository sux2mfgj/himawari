#include <acpi.h>
#include <hm/acpi.h>
// #include <hm/madt.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/vmm.h>

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

static void dump_sdt_header(struct sdt_header_t *hdr) {
  char tmp[5];
  memcpy(tmp, hdr->signature, 4);
  tmp[4] = '\0';
  kprintf("acpi: sdt: sig %s\n", tmp);
  kprintf("acpi: sdt: rev %d\n", hdr->reivison);
}

static int parse_sdt_32(struct rsdt_t *rsdt) {

  int nentry = (rsdt->header.length - sizeof(*rsdt)) / sizeof(rsdt->entry[0]);

  int ret;
  for (int i = 0; i < nentry; i++) {
    struct sdt_header_t *hdr = (struct sdt_header_t *)(uintptr_t)rsdt->entry[i];
    dump_sdt_header(hdr);

    if (memcmp(hdr->signature, DESC_TABLE_SIG_MCFG,
               sizeof(DESC_TABLE_SIG_MCFG) - 1)) {
      ret = acpi_table_parse_mcfg(hdr);
      if (ret < 0) {
        kprintf("failed to parse MCFG table\n");
        return -1;
      }
    }

    if (memcmp(hdr->signature, DESC_TABLE_SIG_MADT,
               sizeof(DESC_TABLE_SIG_MADT) - 1)) {
      ret = acpi_table_parse_madt(hdr);
      if (ret < 0) {
        kprintf("failed to parse MADT table\n");
        return -1;
      }
    }
  }

  return 0;
}

int acpi_init(struct rsdp_v1_t *rsdp) {
  int ret;

  if (!memcmp(rsdp->signature, RSDP_SIGNATURE, sizeof(RSDP_SIGNATURE) - 1))
    return -1;

  void *root_sdt = NULL;
  switch (rsdp->revision) {
  case RSDP_REV_ACPI_1:
    kprintf("ACPI rev 1\n");
    root_sdt = (void *)(uint64_t)rsdp->rsdt_address;

    struct mem_block block = {
        .base = (uint64_t)root_sdt & ~(0x1000 - 1),
        .npages = 1,
    };
    vmm_map_ram_straight(&block);

    if (!validate_rsdt(root_sdt))
      return false;
    break;
  case RSDP_REV_ACPI_2: {
    kprintf("ACPI rev 2\n");
    struct rsdp_v2_t *rsdp_v2 = (struct rsdp_v2_t *)rsdp;
    root_sdt = (void *)rsdp_v2->xsdt_address;

    struct mem_block block = {
        .base = (uint64_t)root_sdt & ~(0x1000 - 1),
        .npages = 1,
    };
    vmm_map_ram_straight(&block);

    if (!validate_xsdt((struct xsdt_t *)root_sdt))
      return false;

    return -1;
    break;
  }
  default:
    kprintf("unknown revision type found\n");
    return -1;
  }

  ret = parse_sdt_32(root_sdt);
  if (ret < 0)
    return ret;

  return 0;
}
