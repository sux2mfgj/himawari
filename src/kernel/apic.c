#include "apic.h"
#include "cpuid.h"

// 10.4.2 Presence of the Local APIC
bool is_presence_local_apic(void)
{
    uint32_t edx;
    cpuid(1, &edx);

    if(edx & 0x100)
    {
        return true;
    }

    return false;
}

bool init_apic(void)
{
    bool result;

    result = is_presence_local_apic();
    if(!result)
    {
        return false;
    }

    //TODO

    return true;
}
