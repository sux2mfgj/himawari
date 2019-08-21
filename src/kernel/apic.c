#include "apic.h"
#include "cpuid.h"
#include "msr.h"
#include "utils.h"

enum IA32_APIC_BASE_BITS {
    IA32_APIC_BASE_BSP  = 0x100,
    IA32_APIC_BASE_GLOBAL_ENABLE = 0x800,
    IA32_APIC_BASE_APIC_BASE = ~0xfff,
};

static uintptr_t bsp_local_apic_base;

// 10.4.2 Presence of the Local APIC
static bool is_presence_local_apic(void)
{
    uint32_t edx;
    cpuid(1, &edx);

    // TODO remove the magic number
    if(edx & 0x100)
    {
        return true;
    }

    return false;
}

// 10.4.3 Enabling or Disabling the Local APIC
static bool is_enable_local_apic(void)
{
    uint64_t apic_base = read_msr(IA32_APIC_BASE);
    // TODO remove the magic number
    return !!(apic_base & IA32_APIC_BASE_GLOBAL_ENABLE);
}

static void enable_local_apic(void)
{
    uint64_t apic_base = read_msr(IA32_APIC_BASE);
    apic_base |= IA32_APIC_BASE_GLOBAL_ENABLE;
    uint32_t low = (uint32_t)apic_base;
    uint32_t high = apic_base >> 32;
    write_msr(IA32_APIC_BASE, high, low);
}

static void setup_bsp_local_apic_base(void)
{
    uint64_t apic_base = read_msr(IA32_APIC_BASE);
    if(!(apic_base & IA32_APIC_BASE_BSP))
    {
        assert(false, "not yet implemented for MP system");
    }

    bsp_local_apic_base = apic_base & IA32_APIC_BASE_APIC_BASE;
}

bool init_apic(void)
{
    bool result;

    result = is_presence_local_apic();
    if(!result)
    {
        return false;
    }

    result = is_enable_local_apic();

    if(!result)
    {
        enable_local_apic();
    }

    setup_bsp_local_apic_base();

    //TODO

    return true;
}
