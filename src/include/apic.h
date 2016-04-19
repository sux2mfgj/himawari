#pragma once

#include <stdint.h>

enum APIC_MAP{
    //TODO
    //  if you need address, you should add values from Intel developer's manual.
    LOCAL_APIC_ID           = 0x20,     // read/write
    LOCAL_APIC_VERSION      = 0x30,     // read only

    LOCAL_APIC_SPURIOUS_IVR     = 0xf0, // read/write

    LOCAL_APIC_LVT_TIMER        = 0x320,    // read/write
    LOCAL_APIC_INITIAL_COUNT    = 0x380,    // read/write
    LOCAL_APIC_CURRENT_COUNT    = 0x390,    // read only
    LOCAL_APIC_DIVIDE_CONFIG    = 0x3e0,    // read/write
};

enum {
    TIMER_MODE_ONE_SHOT = 0b00UL << 17,
    TIMER_MODE_PERIODIC = 0b01UL << 17,
    TIMER_MODE_TSC_DEADLINE = 0b10UL << 17,

    TIMER_MASKED    = 0x1 << 16,
    TIMER_DELIVERY_SEND = 0x1 << 12,
};
