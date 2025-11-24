#include <hm/pci.h>
#include <hm/print.h>

#define PCI_DEV_NUM_MAX 128
static int pci_dev_num = 0;
static struct pci_device devices[PCI_DEV_NUM_MAX];

int pci_register_device(uint8_t *config_space) {

  if (pci_dev_num >= PCI_DEV_NUM_MAX)
    return -1;

  struct pci_device *dev = &devices[pci_dev_num];

  *dev = (struct pci_device){
      .cfg_space = config_space,
  };

  pci_dev_num++;

  return 0;
}

static int pci_scan_device(void *base, uint8_t bus, uint8_t dev) {

  uint8_t *addr = (uint8_t *)((uintptr_t)base | (bus << 20) | (dev << 15));

  uint16_t vendor_id = *(uint16_t *)(addr + 0);
  uint16_t device_id = *(uint16_t *)(addr + 2);

  if (vendor_id == 0xffff)
    return 0;

  kprintf("Found pci device(%d:%d) : vendor 0x%x, device 0x%x\n", bus, dev,
          vendor_id, device_id);

  // TODO: check the device type and etc.

  return 0;
}

#define PCI_DEV_MAX 32
int pci_scan_bus(void *base, uint8_t start_bus, uint8_t end_bus) {
  int ret;
  for (int bus = start_bus; bus <= end_bus; bus++) {
    for (int dev = 0; dev < PCI_DEV_MAX; dev++) {
      ret = pci_scan_device(base, bus, dev);
      if (ret < 0) {
        kprintf("failed to scan device: bus %s, dev %d\n", bus, dev);
        return ret;
      }
    }
  }

  return 0;
}
