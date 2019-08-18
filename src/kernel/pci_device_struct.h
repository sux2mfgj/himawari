#pragma once

typedef struct {
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
    uint8_t header_type;
    uint8_t bist;
    uint64_t base_address_register_0;
    uint64_t base_address_register_1;
    uint64_t base_address_register_2;
    //TODO
} pci_config_space_t;
