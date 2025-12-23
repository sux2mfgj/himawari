#pragma once

#include <pcie.h>
#include <stdint.h>

struct msix_capability {
  struct pcie_cap base;
  uint16_t message_control;
  uint32_t table_offset;
  uint32_t pba_offset;
} __attribute__((packed));

struct msix_table_entry {
  uint32_t message_address_lo;
  uint32_t message_address_hi;
  uint32_t message_data;
  uint32_t vector_control;
} __attribute__((packed));

struct msix {
  struct msix_capability *cap;
  struct msix_table_entry *table;
  uint64_t *pending_bit_array;
};

#define MSIX_BAR_MASK 0b111
#define MSIX_OFFSET_MASK ~MSIX_BAR_MASK
