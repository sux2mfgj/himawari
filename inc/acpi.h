#pragma once

#include <stdint.h>

struct rsdp_v1_t {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_address;
} __attribute__((packed));

struct rsdp_v2_t {
  struct rsdp_v1_t v1;
  uint32_t length;
  uint64_t xsdt_address;
  uint8_t extended_checksum;
  uint8_t reserved[3];
} __attribute__((packed));

#define RSDP_SIGNATURE "RSD PTR "
#define RSDP_REV_ACPI_1 0
#define RSDP_REV_ACPI_2 2

struct sdt_header_t {
  char signature[4];
  uint32_t length;
  uint8_t reivison;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));

struct xsdt_t {
  struct sdt_header_t header;
  uint64_t entry[];
} __attribute__((packed));

struct rsdt_t {
  struct sdt_header_t header;
  uint32_t entry[];
};

#define DESC_TABLE_SIG_XSDT "XSDT"
#define DESC_TABLE_SIG_RSDT "RSDT"
#define DESC_TABLE_SIG_MCFG "MCFG"
#define DESC_TABLE_SIG_MADT "APIC"
#define DESC_TABLE_SIG_HPET "HPET"
