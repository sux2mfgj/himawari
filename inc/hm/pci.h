#pragma once

#include <hm/device.h>
#include <stdint.h>

/* PCI Configuration Space Registers */
#define PCI_CONFIG_VENDOR_ID 0x00
#define PCI_CONFIG_DEVICE_ID 0x02
#define PCI_CONFIG_COMMAND 0x04
#define PCI_CONFIG_STATUS 0x06
#define PCI_CONFIG_REVISION_ID 0x08
#define PCI_CONFIG_PROG_IF 0x09
#define PCI_CONFIG_SUBCLASS 0x0A
#define PCI_CONFIG_CLASS_CODE 0x0B
#define PCI_CONFIG_HEADER_TYPE 0x0E
#define PCI_CONFIG_BAR0 0x10
#define PCI_CONFIG_BAR1 0x14
#define PCI_CONFIG_BAR2 0x18
#define PCI_CONFIG_BAR3 0x1C
#define PCI_CONFIG_BAR4 0x20
#define PCI_CONFIG_BAR5 0x24

/* Header Type flags */
#define PCI_HEADER_TYPE_MASK 0x7F
#define PCI_HEADER_TYPE_MULTIFUNCTION 0x80

/* Invalid vendor ID */
#define PCI_VENDOR_INVALID 0xFFFF

struct pcie_device {
  struct device dev;
  void *config_space;
  struct msix *msix;
};

/* Function prototypes */
int pci_register_ecam(void *base, uint8_t start_bus, uint8_t end_bus);

uint8_t pci_read_config_byte(void *config_space, uint8_t offset);
uint16_t pci_read_config_word(void *config_space, uint8_t offset);
uint32_t pci_read_config_dword(void *config_space, uint8_t offset);
void pci_write_config_byte(void *config_space, uint8_t offset, uint8_t value);
void pci_write_config_word(void *config_space, uint8_t offset, uint16_t value);
void pci_write_config_dword(void *config_space, uint8_t offset, uint32_t value);

int pci_get_bar(struct pcie_device *pdev, int idx, uint64_t *bar,
                uint64_t *size);

void pci_enable_bus_master(struct pcie_device *pdev);

#define VIRTIO_PCIE_CAP_ID 0x09
#define MSIX_PCIE_CAP_ID 0x11

/* PCI Command Register bits */
#define PCI_COMMAND_IO 0x0001
#define PCI_COMMAND_MEMORY 0x0002
#define PCI_COMMAND_MASTER 0x0004
#define PCI_COMMAND_SPECIAL 0x0008
#define PCI_COMMAND_INVALIDATE 0x0010
#define PCI_COMMAND_VGA_PALETTE 0x0020
#define PCI_COMMAND_PARITY 0x0040
#define PCI_COMMAND_WAIT 0x0080
#define PCI_COMMAND_SERR 0x0100
#define PCI_COMMAND_FAST_BACK 0x0200
#define PCI_COMMAND_INTX_DISABLE 0x0400
