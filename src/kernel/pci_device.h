#pragma once

#include <stdint.h>

typedef struct
{
    uintptr_t base_address;
    uint16_t segment_group_number;
    uint8_t start_bus_number;
    uint8_t end_bus_number;

    //TODO
} pci_device_t;

