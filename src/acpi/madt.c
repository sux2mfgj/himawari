#include <acpi.h>
#include <hm/apic.h>
#include <hm/core.h>
#include <hm/print.h>
#include <hm/string.h>

struct madt {
  struct sdt_header_t hdr;
  uint32_t local_apci_base;
  uint32_t flags;
  uint8_t entries[];
} __attribute__((packed));

struct entry_hdr {
  uint8_t type;
  uint8_t length;
} __attribute__((packed));

#define MADT_ENTRY_TYPE_LOCAL_APIC (0)
#define MADT_ENTRY_TYPE_IO_APIC (1)
#define MADT_ENTRY_TYPE_IO_APIC_INT_SRC_OVERRIDE (2)
#define MADT_ENTRY_TYPE_IO_NMI_SRC (3)
#define MADT_ENTRY_TYPE_LOCAL_APIC_NMI_SRC (4)
#define MADT_ENTRY_TYPE_LOCAL_APIC_OVERRIDE (5)
#define MADT_ENTRY_TYPE_LOCAL_x2_APIC (6)

struct local_apic_entry {
  struct entry_hdr hdr;
  uint8_t acpi_processor_id;
  uint8_t apic_id;
#define LOCAL_APIC_ENTRY_FLAGS_PROC_ENABLE (0b1 << 0)
#define LOCAL_APIC_ENTRY_FLAGS_ONLINE_CAP (0b1 << 1)
  uint32_t flasg;
} __attribute__((packed));

struct io_apic_entry {
  struct entry_hdr hdr;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t base;
  uint32_t global_int_base;
} __attribute__((packed));

struct io_apic_src_ovverride_entry {
  struct entry_hdr hdr;
  uint8_t bus_src;
  uint8_t irq_src;
  uint32_t glocal_system_interrupt;
  uint16_t flags;
} __attribute__((packed));

int acpi_table_parse_madt(struct sdt_header_t *hdr) {

  int ret;

  if (!memcmp(hdr->signature, DESC_TABLE_SIG_MADT,
              sizeof(DESC_TABLE_SIG_MADT) - 1))
    return -1;

  struct madt *madt = (struct madt *)hdr;

  kprintf("Local APIC 0x%x\n", madt->local_apci_base);

  struct local_apic *lapic;
  ret = local_apic_init(madt->local_apci_base, &lapic);
  if (ret) {
    kprintf("Failed to init local apic\n");
    return ret;
  }

  uint32_t bsp_id;
  ret = local_apic_read_id(lapic, &bsp_id);
  if (ret) {
    kprintf("failed to get local apic id", bsp_id);
    return ret;
  }
  ret = core_init(bsp_id);
  if (ret) {
    kprintf("failed to set boot core id: %d", bsp_id);
    return ret;
  }

  size_t remain = madt->hdr.length - sizeof(*madt);
  kprintf("remain %d\n", remain);

  struct entry_hdr *entry_hdr = (struct entry_hdr *)madt->entries;
  while (remain) {

    kprintf("type %d, length %d\n", entry_hdr->type, entry_hdr->length);

    switch (entry_hdr->type) {
    case MADT_ENTRY_TYPE_LOCAL_APIC: {
      struct local_apic_entry *lapic = (struct local_apic_entry *)entry_hdr;
      kprintf("Local APIC: proc id %d, apic id %d, flags 0x%x\n",
              lapic->acpi_processor_id, lapic->apic_id, lapic->flasg);

      ret = core_register_app_core_id(lapic->apic_id);
      if (ret) {
        kprintf("Failed to set core id: %d", lapic->apic_id);
        return ret;
      }

      break;
    }
    case MADT_ENTRY_TYPE_IO_APIC: {
      struct io_apic_entry *ioapic = (struct io_apic_entry *)entry_hdr;
      kprintf("IO APIC: io apic id %d, base 0x%x\n", ioapic->io_apic_id,
              ioapic->base);
      break;
    }
    case MADT_ENTRY_TYPE_IO_APIC_INT_SRC_OVERRIDE: {
      struct io_apic_src_ovverride_entry *io_apic_src_override =
          (struct io_apic_src_ovverride_entry *)entry_hdr;
      kprintf("IO APIC int override: bus %d, irq %d\n",
              io_apic_src_override->bus_src, io_apic_src_override->irq_src);

      break;
    }
    default:
      kprintf("unhandled %d\n", entry_hdr->type);
    }

    remain -= entry_hdr->length;
    entry_hdr = (struct entry_hdr *)((uint8_t *)entry_hdr + entry_hdr->length);
  }

  return 0;
}
