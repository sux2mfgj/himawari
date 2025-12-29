#pragma once

#include <stdint.h>

struct pcie_cap {
  uint8_t cap_id;
  uint8_t cap_next;
} __attribute__((packed));
