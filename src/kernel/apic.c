#include "apic.h"
#include "cpuid.h"
#include "msr.h"

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
    return !!(apic_base & 0x800);
}

static void enable_local_apic(void)
{
    uint64_t apic_base = read_msr(IA32_APIC_BASE);
    apic_base |= 0x800;
    uint32_t low = (uint32_t)apic_base;
    uint32_t high = apic_base >> 32;
    write_msr(IA32_APIC_BASE, high, low);
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


    //TODO

    return true;
}
