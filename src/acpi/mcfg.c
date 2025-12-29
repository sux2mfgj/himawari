#include <acpi.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/vm.h>

struct acpi_mcfg_record {
  uint64_t base_addr;
  uint16_t pci_segment_group_number;
  uint8_t start_bus_number;
  uint8_t end_bus_number;
  uint32_t reserved;
} __attribute__((packed));

int acpi_table_parse_mcfg(struct sdt_header_t *hdr) {

  if (memcmp(hdr->signature, DESC_TABLE_SIG_MCFG,
             sizeof(DESC_TABLE_SIG_MCFG) - 1))
    return -1;

  // MCFG table has 8 bytes reserved after header before first record
  int nentry = (hdr->length - sizeof(*hdr) - 8) / sizeof(struct acpi_mcfg_record);
  struct acpi_mcfg_record *record =
      (struct acpi_mcfg_record *)((uintptr_t)hdr + sizeof(*hdr) + 8);

  int ret;
  for (int i = 0; i < nentry; i++) {
    kprintf("acpi: mcfg[%d]: base 0x%lx, seg %d, bus %d-%d\n", i,
            record->base_addr, record->pci_segment_group_number,
            record->start_bus_number, record->end_bus_number);

    // 256 MiB(range)
    vm_map_device_straight(record->base_addr, (256 * 1024 * 1024) / 0x1000);

    ret = pci_register_ecam((void *)record->base_addr, record->start_bus_number,
                            record->end_bus_number);

    if (ret < 0)
      return ret;
  }

  return 0;
}
