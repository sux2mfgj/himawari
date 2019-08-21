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
    uint64_t reserved;
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
    // 131 is a offset of FADT Minor Version in the FACP table.
    uint8_t dummy[131 - sizeof(acpi_dt_header_t)];
    uint8_t fadt_minor_version;
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;
    // TODO there are many entries.
}__attribute__((__packed__)) acpi_fadt_t;

typedef struct {
    acpi_dt_header_t header;
    uint8_t definition_block[];
}__attribute__((__packed__)) acpi_dsdt_t;

typedef struct __config_space_based_address_alloc_t {
    uint64_t base_address;
    uint16_t pci_segment_group_number;
    uint8_t start_pci_bus_number;
    uint8_t end_pci_bus_number;
    uint32_t reserved;
} __attribute__((__packed__)) pci_config_info_t;

typedef struct {
    acpi_dt_header_t header;
    pci_config_info_t config_space[];
}__attribute__((__packed__)) acpi_mcfg_t;

