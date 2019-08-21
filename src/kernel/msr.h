#pragma once

#include <stdint.h>

enum
{
    IA32_APIC_BASE = 0x1b
};

uint64_t read_msr(uint32_t msr);
void write_msr(uint32_t msr, uint32_t high, uint32_t low);
