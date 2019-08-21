#pragma once

enum PCI_HEADER_TYPE {
    PCI_HEADER_TYPE_STANDART = 0,
    PCI_HEADER_TYPE_PCI_TO_PCI = 1,
    PCI_HEADER_TYPE_CARDBUS = 2,
};

typedef struct
{
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t program_interface;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    union {
        uint8_t header_type;
        struct
        {
            unsigned type : 7;
            unsigned multi_function : 1;
        } __attribute__((__packed__));
    };
    uint8_t bist;

} pci_config_first_part_t;

typedef struct
{
    pci_config_first_part_t first_part;
    uint64_t base_address_0;
    uint64_t base_address_1;
    uint64_t base_address_2;
    uint32_t cardbus_cis_pointer;
    struct {
        uint16_t vendor_id;
        uint16_t system_id;
    } subsystem;
    uint32_t expansion_rom_base_address;
    uint16_t capabilities_pointer;
    uint16_t reserved_0;
    uint32_t reserved_1;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint8_t min_grant;
    uint8_t max_latency;
} pci_config_space_standard_t;

typedef struct
{
    pci_config_first_part_t first_part;
    uint64_t base_address_0;
    uint8_t primary_bus_number;
    uint8_t secondary_bus_number;
    uint8_t subordinate_bus_number;
    uint8_t secondary_latency_timer;
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    //TODO
} pci_config_space_pci_to_pci_t;

typedef struct
{
    pci_config_first_part_t first_part;
    //TODO
} pci_config_space_cardbus_bridge;

