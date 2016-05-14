#include <init.h>
#include <descriptor.h>
#include <segment.h>
#include <page.h>
#include <x86_64.h>
#include <trap.h>
#include <string.h>
#include <task_call.h>

extern struct descriptor_ptr idt_desc;

/* uint32_t* idt_table; */

bool init_trap(void)
{

    // idt_table = (struct gate_descriptor*)early_malloc(1);
    // make_gate(idt_table, 64, vectors[64] - (uintptr_t)START_KERNEL_MAP, 3,
    // GATE_TRAP);

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

    // TODO setup for MSR

    return true;
}

static char *fault_list[] = {
    "DE", "DB", "NMI", "BP", "OF", "BR", "UD", "NM", "DF", "CSO",
    "TS", "NP", "SS",  "GP", "PF", "MF", "AC", "MC", "XM", "VE",
};

void trap(struct trap_frame_struct *trap_frame)
{
    switch (trap_frame->trap_number)
    {
        case IDT_ENTRY_PIC_TIMER:
        {
            schedule(trap_frame);
            irq_eoi();
            break;
        }

        case IDT_ENTRY_TASK_CALL:
        {
            task(trap_frame);
            break;
        }
        default:
        {
            while (true)
            {
            }
            break;
        }
    }

    // when page fault occured, you should read cr2 and print display.
    // this is setted about try to access memory addres
}
