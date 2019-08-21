#pragma once

#include <stdbool.h>
#include <stdint.h>

bool init_acpi(uintptr_t rsdp);
bool get_mcfgt_info(int index, uintptr_t *base_address,
                    uint16_t *segment_group_number, uint8_t *start_bus_number,
                    uint8_t *end_bus_number);
