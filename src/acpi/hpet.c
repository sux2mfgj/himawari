#include <acpi.h>
#include <hm/acpi.h>
#include <hm/print.h>
#include <hm/string.h>
#include <hm/vm.h>

struct acpi_address {
  uint8_t addr_space_id;
  uint8_t register_bit_width;
  uint8_t register_bit_offset;
  uint8_t reserved;
  uint64_t address;
} __attribute__((packed));

struct hpet_table {
  struct sdt_header_t hdr;
  uint8_t revision;
  uint8_t comp_count;
  uint8_t counter_size;
  uint8_t reserved;
  struct acpi_address address;
  uint8_t number;
  uint16_t minimum_clock_tick;
  uint8_t oem_attr;
} __attribute__((packed));

#define REG_OFFSET_GCID (0x0)
#define REG_OFFSET_RSV0 (0x8)
#define REG_OFFSET_GC (0x10)
#define REG_GC_ENABLE (1 << 0)
#define REG_GC_LEGACY_MAP (1 << 1)

#define REG_OFFSET_GIS (0x18)
#define REG_OFFSET_MC (0xf0)

#define COMP_REG_OFFSET_CONFIG (0x0)
#define COMP_REG_OFFSET_COMP_VAL (0x8)
#define COMP_REG_OFFSET_FSB_ROUTE (0x10)

static int hpet_init(uint8_t *reg_base) {

  volatile uint64_t *reg_gc = (uint64_t *)(reg_base + REG_OFFSET_GC);
  volatile uint64_t *reg_mc = (uint64_t *)(reg_base + REG_OFFSET_MC);

  uint64_t val = *reg_gc;
  kprintf("hpet: gc 0x%x\n", *reg_gc);
  *reg_gc = val;
  kprintf("hpet: gc 0x%x\n", *reg_gc);

  kprintf("hpet: mc 0x%x\n", *reg_mc);
  *reg_mc = 0;
  kprintf("hpet: mc 0x%x\n", *reg_mc);

  return 0;
}

int acpi_table_parse_hpet(struct sdt_header_t *hdr) {

  int ret;

  if (memcmp(hdr->signature, DESC_TABLE_SIG_HPET,
             sizeof(DESC_TABLE_SIG_HPET) - 1))
    return -1;

  struct hpet_table *hpet = (struct hpet_table *)hdr;

  kprintf("hpet: reg base 0x%x\n", hpet->address.address);
  uint8_t *reg_base = (uint8_t *)hpet->address.address;

  kprintf("hpet: rev %d, comparators %d\n", hpet->revision, hpet->comp_count);

  vm_map_device_straight((uint64_t)reg_base, 1);

  ret = hpet_init(reg_base);
  if (ret < 0)
    return ret;

  kprintf("hpet: setup completed\n");

  return 0;
}
