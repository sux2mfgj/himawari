#include <init.h>
#include <x86_64.h>
#include <page.h>
#include <apic.h>
#include <descriptor.h>
#include <kernel.h>

static uintptr_t local_apic_addr = 0;

static uint32_t apic_read(enum APIC_MAP map) 
{
    return *(uint32_t*)(local_apic_addr + LOCAL_APIC_ID);
}

static void apic_write(enum APIC_MAP map, uint32_t value)
{
   *(uint32_t*)(local_apic_addr + map) = value;
   //wait for next, by reading ID
   volatile uint32_t id = apic_read(LOCAL_APIC_ID);
   nothing((void *)&id);
}

bool init_local_apic(void)
{
    uint32_t eax, edx;
    cpuid(1, &eax, &edx);

    {
        char buf[32];
        itoa((uint64_t)edx, buf, 16);
        puts("cpuid: 0x");
        puts(buf);
        puts("\n");
    }

    if(!(edx & CPUID_FLAG_APIC)) {
        return false;
    }

    uintptr_t new_apic_phys_base_addr = early_malloc(1);

    local_apic_addr = new_apic_phys_base_addr;
    new_apic_phys_base_addr -= START_KERNEL_MAP;

    uint32_t apic_msr_value = 
        (new_apic_phys_base_addr & 0xfffffffffffff000) |
        APIC_MSR_BSP |
        APIC_MSR_GLOBAL_ENABLE;

    //XXX APIC_MSR_BSP (bootstrap processor)
    {
        char buf[32];
        itoa((uint64_t)apic_msr_value, buf, 16);
        puts("new msr value: 0x");
        puts(buf);
        puts("\n");
    }


    eax = apic_msr_value;
    edx = 0;
    write_msr(IA32_APIC_BASE_MSR, &eax, &edx);

    //setup local apic timer
    // stop local APIC timer
    apic_write(LOCAL_APIC_INITIAL_COUNT, 0);
    apic_write(LOCAL_APIC_DIVIDE_CONFIG, 0x3); // divieded by 16

    uint64_t lvt_timer_value 
        = TIMER_MODE_PERIODIC | TIMER_DELIVERY_SEND | IDT_ENTRY_TIMER;

    apic_write(LOCAL_APIC_LVT_TIMER, lvt_timer_value);

    //TODO setup 10ms by using pit
    // 
    apic_write(LOCAL_APIC_INITIAL_COUNT, 10000000);

    return true;
}


