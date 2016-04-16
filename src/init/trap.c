#include <init.h>
#include <descriptor.h>
#include <segment.h>

struct gate_descriptor idt_table[IDT_ENTRY_NUM];
extern uintptr_t vectors[VECTOR_ENTRY_NUM];

void set_gate_descriptor(int descriptor_num,
                         unsigned type,
                         uintptr_t offset,
                         unsigned ist,
                         unsigned dpl)
{
    idt_table[descriptor_num].offset_low = offset & 0xffff;
    idt_table[descriptor_num].segment_selector = KERNEL_CODE_SEGMENT;
    idt_table[descriptor_num].ist = ist;
    idt_table[descriptor_num].zero5 = 0;
    idt_table[descriptor_num].type = type;
    idt_table[descriptor_num].zero1 = 0;
    idt_table[descriptor_num].dpl = dpl;
    idt_table[descriptor_num].segment_present = 1;
    idt_table[descriptor_num].offset_middle = (offset >> 16) & 0xffff;
    idt_table[descriptor_num].offset_high = (offset >> 32) & 0xffffffff;
}

bool init_trap(void)
{
    for (int i = 0; i < IDT_ENTRY_NUM; ++i) {
        set_gate_descriptor(i, GATE_INTERRUPT, vectors[i], 0, 0);
    }

    return true;
}

void trap(uintptr_t stack_head)
{
    while (true) {
    }
}
