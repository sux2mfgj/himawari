#include <stdint.h>
#include <descriptor.h>
#include <x86_64.h>
#include <segment.h>
#include <page.h>
#include <stdbool.h>

extern void start_kernel(uintptr_t bootinfo_addr);

struct descriptor_ptr idt_desc = {256 * 16, (uintptr_t)idt_table + START_KERNEL_MAP};

void early_idt_handler(void)
{
    while (true) {
        
    }
}

struct descriptor_ptr gdt_desc = {8 * 16, (uintptr_t)gdt_table + START_KERNEL_MAP};

/* uint8_t ist_0[0x1000 / 8]; */
/* struct tss_struct init_tss; */
void pre_start_kernel(uintptr_t bootinfo_addr)
{
    for(int i=0; i < 256; ++i)
    {
        set_intr_gate(i,
                (void *)((uintptr_t)early_idt_handler));
    }

/*     struct tss_struct *t = &init_tss; */
/*     t->ist[0] = (uintptr_t)ist_0 + 0x1000; */
/*     t->rsp0 = (uintptr_t)ist_0 + 0x1000; */

    //set_tss_desc((uintptr_t)t);
    
    __asm__ volatile("lgdt %0" :: "m"(gdt_desc));
    __asm__ volatile("lidt %0" :: "m" (idt_desc));
/*     __asm__ volatile("ltr %w0" :: "r" (TASK_STATE_SEGMENT)); */

    start_kernel(bootinfo_addr);
}
