#include <hm/device.h>
#include <hm/mm.h>
#include <hm/pci.h>
#include <hm/print.h>
#include <hm/string.h>
#include <stdint.h>

static void *calc_config_base(void *ecam_base, uint8_t bus, uint8_t device,
                              uint8_t function) {
  uintptr_t base =
      (uintptr_t)ecam_base + ((bus << 20) | (device << 15) | (function << 12));
  return (void *)base;
}

/* Read 8-bit value from PCI configuration space */
uint8_t pci_read_config_byte(void *config_space, uint8_t offset) {
  volatile uint8_t *addr = config_space + offset;
  if (!addr) {
    return 0xFF;
  }

  return *addr;
}

/* Read 16-bit value from PCI configuration space */
uint16_t pci_read_config_word(void *config_space, uint8_t offset) {
  volatile uint16_t *addr = config_space + offset;
  if (!addr) {
    return 0xFFFF;
  }

  return *addr;
}

/* Read 32-bit value from PCI configuration space */
uint32_t pci_read_config_dword(void *config_space, uint8_t offset) {
  volatile uint32_t *addr = config_space + offset;
  if (!addr) {
    return 0xFFFFFFFF;
  }

  return *addr;
}

/* Write 8-bit value to PCI configuration space */
void pci_write_config_byte(void *config_space, uint8_t offset, uint8_t value) {
  volatile uint8_t *addr = config_space + offset;
  if (addr) {
    *addr = value;
  }
}

/* Write 16-bit value to PCI configuration space */
void pci_write_config_word(void *config_space, uint8_t offset, uint16_t value) {
  volatile uint16_t *addr = config_space + offset;
  if (addr) {
    *addr = value;
  }
}

/* Write 32-bit value to PCI configuration space */
void pci_write_config_dword(void *config_space, uint8_t offset,
                            uint32_t value) {
  volatile uint32_t *addr = config_space + offset;
  if (addr) {
    *addr = value;
  }
}

/* Check if a device exists at the given location */
static int pci_device_exists(void *ecam_base, uint8_t bus, uint8_t device,
                             uint8_t function) {
  void *config_base = calc_config_base(ecam_base, bus, device, function);
  uint16_t vendor_id = pci_read_config_word(config_base, PCI_CONFIG_VENDOR_ID);
  return vendor_id != PCI_VENDOR_INVALID;
}

/* Scan a specific function */

static char *gen_pci_device_name(uint16_t vendor_id, uint16_t device_id,
                                 uint8_t bus, uint8_t device,
                                 uint8_t function) {
  size_t size = sizeof("PCI VVVV:DDDD (BB:DD:FF)");
  char *name = mm_alloc(size);

  snprintf(name, size, "PCI %x:%x (%x:%x:%x)", vendor_id, device_id, bus,
           device, function);

  return name;
}

static int pci_register_device(void *ecam_base, uint8_t bus, uint8_t device,
                               uint8_t function) {

  struct pcie_device *pdev = mm_alloc(sizeof(*pdev));

  void *config_base = calc_config_base(ecam_base, bus, device, function);

  uint16_t vendor_id = pci_read_config_word(config_base, PCI_CONFIG_VENDOR_ID);
  uint16_t device_id = pci_read_config_word(config_base, PCI_CONFIG_DEVICE_ID);

  pdev->dev = (struct device){
      .type = PCIE,
      .match.pcie =
          {
              .vendor_id = vendor_id,
              .device_id = device_id,
          },
      .name = gen_pci_device_name(vendor_id, device_id, bus, device, function),
  };

  pdev->config_space = config_base;

  return register_device(&pdev->dev);
}

/* Scan a specific device (all functions) */
static void pci_scan_device(void *ecam_base, uint8_t bus, uint8_t device) {
  if (!pci_device_exists(ecam_base, bus, device, 0))
    return;

  void *config_base = calc_config_base(ecam_base, bus, device, 0);

  /* Scan function 0 */
  pci_register_device(ecam_base, bus, device, 0);

  /* Check if this is a multi-function device */
  uint8_t header_type =
      pci_read_config_byte(config_base, PCI_CONFIG_HEADER_TYPE);

  if (header_type & PCI_HEADER_TYPE_MULTIFUNCTION) {
    /* Scan functions 1-7 */
    for (uint8_t function = 1; function < 8; function++) {
      if (pci_device_exists(ecam_base, bus, device, function))
        pci_register_device(ecam_base, bus, device, function);
    }
  }
}

/* Scan a specific bus (all devices) */
static void pci_scan_bus(void *ecam_base, uint8_t bus) {
  for (uint8_t device = 0; device < 32; device++) {
    pci_scan_device(ecam_base, bus, device);
  }
}

int pci_register_ecam(void *ecam_base, uint8_t start_bus, uint8_t end_bus) {
  if (start_bus) {
    kprintf("no support the start_bus is not started from 0\n");
    return -1;
  }

  if (!ecam_base || start_bus > end_bus) {
    return -1;
  }

  for (int16_t bus = start_bus; bus <= end_bus; bus++) {
    pci_scan_bus(ecam_base, bus);
  }

  return 0;
}

#define PCI_BAR_TYPE_MASK 0x01
#define PCI_BAR_TYPE_MEM 0x00
#define PCI_BAR_TYPE_IO 0x00

#define PCI_BAR_MEM_TYPE_MASK 0x06
#define PCI_BAR_MEM_TYPE_32BIT 0x00
#define PCI_BAR_MEM_TYPE_64BIT 0x04

static int pci_bar_is_memory(uint32_t bar) {
  return (bar & PCI_BAR_TYPE_MASK) == PCI_BAR_TYPE_MEM;
}

static int pci_bar_is_64bit(uint32_t bar) {
  if (!pci_bar_is_memory(bar))
    return 0;

  return (bar & PCI_BAR_MEM_TYPE_MASK) == PCI_BAR_MEM_TYPE_64BIT;
}

int pci_get_bar(struct pcie_device *pdev, int idx, uint64_t *bar,
                uint64_t *size) {
  uint8_t bar_offset = PCI_CONFIG_BAR0 + idx * 4;

  uint32_t bar_lo = pci_read_config_dword(pdev->config_space, bar_offset);

  if (!pci_bar_is_64bit(bar_lo)) {
    // 32-bit BAR
    *bar = (uint64_t)(bar_lo & ~0xF);

    if (size) {
      // Save original value
      uint32_t original = bar_lo;

      // Write all 1s
      pci_write_config_dword(pdev->config_space, bar_offset, 0xFFFFFFFF);

      // Read back
      uint32_t readback = pci_read_config_dword(pdev->config_space, bar_offset);

      // Restore original
      pci_write_config_dword(pdev->config_space, bar_offset, original);

      // Calculate size
      readback &= ~0xF;  // Mask off lower 4 bits
      *size = (~readback) + 1;
    }

    return 0;
  }

  // 64-bit BAR
  uint8_t bar_offset_hi = PCI_CONFIG_BAR0 + (idx + 1) * 4;
  uint32_t bar_hi = pci_read_config_dword(pdev->config_space, bar_offset_hi);

  uint64_t bar64 = (uint64_t)(bar_lo & ~0xF);
  bar64 |= (uint64_t)bar_hi << 32;
  *bar = bar64;

  if (size) {
    // Save original values
    uint32_t original_lo = bar_lo;
    uint32_t original_hi = bar_hi;

    // Write all 1s to both parts
    pci_write_config_dword(pdev->config_space, bar_offset, 0xFFFFFFFF);
    pci_write_config_dword(pdev->config_space, bar_offset_hi, 0xFFFFFFFF);

    // Read back
    uint32_t readback_lo = pci_read_config_dword(pdev->config_space, bar_offset);
    uint32_t readback_hi = pci_read_config_dword(pdev->config_space, bar_offset_hi);

    // Restore original values
    pci_write_config_dword(pdev->config_space, bar_offset, original_lo);
    pci_write_config_dword(pdev->config_space, bar_offset_hi, original_hi);

    // Calculate size
    uint64_t size64 = (uint64_t)(readback_lo & ~0xF);
    size64 |= (uint64_t)readback_hi << 32;
    *size = (~size64) + 1;
  }

  return 0;
}
