#include "trap.h"
#include "graphic.h"
#include "func.h"
#include "timer.h"

static gate_descriptor idt[INTERRUPT_DESCRIPTOR_TABLE_SIZE];

static gate_vector_table exception_table[] = {
    {divide_error, DIVIDE_VECTOR, PRIVILEGE_OS},
    {debug_fault, DEBUG_VECTOR, PRIVILEGE_OS},
    {nmi_interrupt, NMI_VECTOR, PRIVILEGE_OS},
    {break_point, BRAKEPOINT_VECTOR, PRIVILEGE_OS},
    {over_flow, OVERFLOW_VECTOR, PRIVILEGE_OS},
    {br_bound, BR_BOUND_VECTOR, PRIVILEGE_OS},
    {invalid_op, INVALID_OP_VECTOR, PRIVILEGE_OS},
    {double_fault, DOUBLE_FAULT, PRIVILEGE_OS},
    {coproc_segment_overrun, COPROC_SEGMENT_VECTOR, PRIVILEGE_OS},
    {invalid_tss, INVALID_TSS_VECTOR, PRIVILEGE_OS},
    {segment_not_fault, SEG_NOT_VECTOR, PRIVILEGE_OS},
    {stack_exception, STACK_EXCEPTION_VECTOR, PRIVILEGE_OS},
    {general_protection, GENERAL_PROTECTION_VECTOR, PRIVILEGE_OS},
    {page_fault, PAGE_FAULT_VECTOR, PRIVILEGE_OS},
    {coproc_err, COPROC_ERROR_VECOTR, PRIVILEGE_OS},
    {alignment_check, ALIGNMENT_CHECK_VECTOR, PRIVILEGE_OS},
    {machine_check, MACHINE_CHECK_VECTOR, PRIVILEGE_OS},
    {simd_exception, SIMD_EXCEPTION_VECTOR, PRIVILEGE_OS},
    {NULL, 0, 0}};

static gate_vector_table pic_table[] = {
    {hardware_interrupt0, 32, PRIVILEGE_OS},
    {hardware_interrupt1, HARDWARE_VECTOR(1), PRIVILEGE_OS},
    {hardware_interrupt2, HARDWARE_VECTOR(2), PRIVILEGE_OS},
    {hardware_interrupt3, HARDWARE_VECTOR(3), PRIVILEGE_OS},
    {hardware_interrupt4, HARDWARE_VECTOR(4), PRIVILEGE_OS},
    {hardware_interrupt5, HARDWARE_VECTOR(5), PRIVILEGE_OS},
    {hardware_interrupt6, HARDWARE_VECTOR(6), PRIVILEGE_OS},
    {hardware_interrupt7, HARDWARE_VECTOR(7), PRIVILEGE_OS},
    {hardware_interrupt8, HARDWARE_VECTOR(8), PRIVILEGE_OS},
    {hardware_interrupt9, HARDWARE_VECTOR(9), PRIVILEGE_OS},
    {hardware_interrupt10, HARDWARE_VECTOR(10), PRIVILEGE_OS},
    {hardware_interrupt11, HARDWARE_VECTOR(11), PRIVILEGE_OS},
    {hardware_interrupt12, HARDWARE_VECTOR(12), PRIVILEGE_OS},
    {hardware_interrupt13, HARDWARE_VECTOR(13), PRIVILEGE_OS},
    {hardware_interrupt14, HARDWARE_VECTOR(14), PRIVILEGE_OS},
    {hardware_interrupt15, HARDWARE_VECTOR(15), PRIVILEGE_OS},
    {NULL, 0, 0}};

static inline void _set_gatedesc(gate_descriptor* gd, uint32_t offset,
                                 uint32_t selector, uint8_t gate_type,
                                 uint8_t storage_segment,
                                 uint8_t descriptor_privilege_level,
                                 uint8_t present)
{
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->gate_type = gate_type & 0xf;
    gd->storage_segment = storage_segment & 0x1;
    gd->descriptor_privilege_level = descriptor_privilege_level & 0x3;
    gd->present = present & 0x1;
    gd->offset_high = (offset >> 16) & 0xffff;

    return;
}

/* set_gatedesc(idt + 8, (uintptr_t)asm_fault_inthandler, */
/*                  KERNEL_CODE_SEGMENT * 8, GATE_TYPE_32BIT_INT, 0, */
/*                  PRIVILEGE_LEVEL_OS, PRESENT); */


void int_gate(gate_vector_table* gvt)
{
    gate_vector_table* current;
    for (current = gvt; current->gate != NULL; current++) {
        _set_gatedesc(&idt[current->vector_num], (uintptr_t)current->gate,
                      KERNEL_CODE_SEGMENT * 8, INTERRUPT_GATE, 0, current->level,
                      PRESENT);
    }
}

// TODO: set default handler that call kernel panic function. kernel panic
// function is not implement yet
void init_interrupt(void)
{
    int_gate(pic_table);
    int_gate(exception_table);

    load_idtr(sizeof(gate_descriptor) * (INTERRUPT_DESCRIPTOR_TABLE_SIZE - 1),
              (uintptr_t)idt);

    return;
}

void exception_handler(int a, int b)
{
    printk(DEBUG1, "in exception_handler %d: %d", a, b);
/*     while (true) { */
        io_hlt();
/*     } */
}

void irq_handler(int irq)
{
    switch (irq) {
        case 0:
            timer_interrupt();
            break;

        case 1: {
            char data;
            data = io_in8(0x0060);
            if (data <= 81) {
                printk(DEBUG2, "%c", data);
            }
            break;
        }
        default:
            break;
    }
    return;
}
