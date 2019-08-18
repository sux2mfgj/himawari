#pragma once

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
    uint64_t base_address_register_0;
    uint64_t base_address_register_1;
    uint64_t base_address_register_2;
    // TODO
} pci_config_space_t;
