#pragma once

#include <stdint.h>

enum {
    GATE_INTERRUPT = 0xE,
    GATE_TRAP = 0xF,
    GATE_CALL = 0xC,
};

#define IDT_ENTRY_NUM 256
#define VECTOR_ENTRY_NUM 256


struct gate_descriptor {
    uint16_t offset_low;
    uint16_t segment_selector;
    unsigned ist    : 3;
    unsigned zero5  : 5;
    unsigned type   : 4;
    unsigned zero1  : 1;
    unsigned dpl    : 2;
    unsigned segment_present : 1;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
};

