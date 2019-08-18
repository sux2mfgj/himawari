#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((__packed__)) acpi_rsdp_t;

typedef struct
{
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oem_table_id;
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((__packed__)) acpi_dt_header_t;

typedef struct {
    acpi_dt_header_t header;
    uint32_t entries[];
} __attribute__((__packed__)) acpi_rsdt_t;

typedef struct{
    acpi_dt_header_t header;
    uint64_t entries[];
} __attribute__((__packed__)) acpi_xsdt_t;

typedef struct {
    acpi_dt_header_t header;
    // space for ACPI 1.0;
    uint8_t dummy[]; // 132 - 36
    // TODO there are many entries.
}__attribute__((__packed__)) acpi_fadt_t;

bool init_acpi(uintptr_t rsdp);
bool get_pcie_configuration_addr(uintptr_t* addr);
bool get_mcfg_table(uintptr_t *ptr);
