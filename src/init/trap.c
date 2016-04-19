#include <init.h>
#include <descriptor.h>
#include <segment.h>
#include <page.h>
#include <x86_64.h>


extern struct descriptor_ptr idt_desc;


/* uint32_t* idt_table; */
extern uintptr_t vectors[VECTOR_ENTRY_NUM];

/* void set_gate_descriptor(int descriptor_num, */
/*                          unsigned type, */
/*                          uintptr_t offset, */
/*                          unsigned ist, */
/*                          unsigned dpl) */
/* { */
/*     idt_table[descriptor_num].offset_low = (uint16_t)(offset & 0xffff); */
/*     idt_table[descriptor_num].segment_selector = KERNEL_CODE_SEGMENT; */
/*     idt_table[descriptor_num].ist = ist; */
/*     idt_table[descriptor_num].zero5 = 0; */
/*     idt_table[descriptor_num].type = type; */
/*     idt_table[descriptor_num].zero1 = 0; */
/*     idt_table[descriptor_num].dpl = dpl; */
/*     idt_table[descriptor_num].segment_present = 1; */
/*     idt_table[descriptor_num].offset_middle = (uint16_t)((offset >> 16) & 0xffff); */
/*     idt_table[descriptor_num].offset_high = (uint32_t)((offset >> 32) & 0xffffffff); */
/*     idt_table[descriptor_num].reserved = 0; */
/* } */

/* static void lidt(struct gate_descriptor *p, int size) */
/* { */
/*     volatile uint16_t descriptor[5]; */

/*     descriptor[0] = size - 1; */
/*     descriptor[1] = (uintptr_t)p; */
/*     descriptor[2] = (uintptr_t)p >> 16; */
/*     descriptor[3] = (uintptr_t)p >> 32; */
/*     descriptor[4] = (uintptr_t)p >> 48; */

/*     __asm__ volatile("lidt (%0)" : : "r" (descriptor)); */
/* } */

/* static void make_gate(uint32_t *idt,  */
/*         int n,  */
/*         uintptr_t kva, */
/*         int pl, */
/*         int type) */
/* { */
/*     uint64_t addr = (uint64_t)kva; */
/*     n *= 4; */
/*     idt[n+0] = (addr & 0xffff) | (KERNEL_CODE_SEGMENT << 16); */
/*     idt[n+1] = (addr & 0xffff0000) | (0x1 << 15) |(type << 8)| ((pl & 3) << 13); */
/*     idt[n+2] = addr >> 32; */
/*     idt[n+3] = 0; */
/* } */

bool init_trap(void)
{

    //idt_table = (struct gate_descriptor*)early_malloc(1);
    //make_gate(idt_table, 64, vectors[64] - (uintptr_t)START_KERNEL_MAP, 3, GATE_TRAP);

    set_intr_gate(0, &divide_error_exception);
    set_intr_gate(1, &debug_exception);
    set_intr_gate(2, &nmi_interrupt);
    set_intr_gate(3, &breakpoint_exception);
    set_intr_gate(4, &overflow_exception);
    set_intr_gate(5, &bound_range_exceeded_exception);
    set_intr_gate(6, &invalid_opecode_exception);
    set_intr_gate(7, &device_not_availabe_exception);
    set_intr_gate(8, &double_fault_exceptin);
    set_intr_gate(9, &coprocessor_segment_overrun);
    set_intr_gate(10, &invalid_tss_exception);
    set_intr_gate(11, &segment_not_present);
    set_intr_gate(12, &stack_fault_exception);
    set_intr_gate(13, &general_protection_exception);
    set_intr_gate(14, &page_fault_exception);
    set_intr_gate(16, &coprocessor_segment_overrun);
    set_intr_gate(17, &alignment_check_exception);


    //TODO setup entry for system call
    
    cli();

    return true;
}

void trap(uintptr_t stack_head)
{
    {
        char buf[32];
        itoa(stack_head, buf, 16);
        puts("stack head: 0x");
        puts(buf);
        puts("\n");
    }
    while (true) {
    }
}
