#pragma once

#include <stdint.h>

struct pci_device {
  uint8_t *cfg_space;
};

struct pci_bus {
  struct pci_device **devices;
};

int pci_scan_bus(void *base, uint8_t start_bus, uint8_t end_bus);
