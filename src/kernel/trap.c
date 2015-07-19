#include "trap.h"
#include "asm.h"
#include "vectors.h"
#include "segment.h"
#include "kernel.h"
#include "process.h"
/* #include "timer.h" */

/* static gate_descriptor idt[INTERRUPT_DESCRIPTOR_TABLE_SIZE]; */
/* extern gate_descriptor idt[INTERRUPT_DESCRIPTOR_TABLE_SIZE]; */

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
    {hardware_interrupt0, HARDWARE_VECTOR(0), PRIVILEGE_OS},
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

static gate_vector_table system_call_vector = {system_call, SYSTEM_CALL_VECTOR,
                                               PRIVILEGE_USER};

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

void int_gate(gate_vector_table* gvt)
{
    gate_vector_table* current;
    for (current = gvt; current->gate != NULL; current++) {
        _set_gatedesc(&idt[current->vector_num], (uintptr_t)current->gate,
                      KERNEL_CODE_SEGMENT_INDEX * 8, INTERRUPT_GATE, 0,
/*                       KERNEL_CODE_SEGMENT * 8, INTERRUPT_GATE, 0, */
                      current->level, PRESENT);
    }
}

void trap_gate(gate_vector_table tb)
{
    _set_gatedesc(&idt[tb.vector_num], (uintptr_t)tb.gate,
                  KERNEL_CODE_SEGMENT_INDEX * 8, TRAP_GATE, 0, tb.level, PRESENT);
}

// TODO: set default handler that call kernel panic function. kernel panic
// function is not implement yet
bool init_interrupt(void)
{
    idt = (gate_descriptor* )IDT_ADDR;
    printk("idt: 0x%x", idt);
    int_gate(pic_table);
    int_gate(exception_table);
    trap_gate(system_call_vector);


    init_pic();
    init_pit();

    load_idtr(sizeof(gate_descriptor) * (NUMBER_OF_IDT - 1),
              (uintptr_t)idt);

    return true;
}

void init_pic(void)
{
    // Mask
    io_out8(PIC_MASTER_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);

    // Master
    io_out8(PIC_MASTER_CMD_STATE_PORT, PIC_MASTER_ICW1);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW2);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW3);
    io_out8(PIC_MASTER_DATA_PORT, PIC_MASTER_ICW4);

    // Slave
    io_out8(PIC_SLAVE_CMD_STATE_PORT, PIC_SLAVE_ICW1);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW2);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW3);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_SLAVE_ICW4);

    // setting enable
    io_out8(PIC_MASTER_DATA_PORT, 0xff);  // 1111 1100
    io_out8(PIC_SLAVE_DATA_PORT, 0xff);   // 1111 1111

    return;
}

void init_pit(void)
{
    set_pit_count(PIT_CLK_10MS,
                  PIT_CONTROL_WORD_SC_COUNTER0,
                  PIT_CONTROL_WORD_MODE_SQARE);

    return;
}

void set_pit_count(uint16_t count, uint8_t counter, uint8_t mode)
{
    uint8_t command;

    command = mode | PIT_CONTROL_WORD_RL_LSB_MSB | counter;

    io_out8(PIT_PORT_CONTROL_WORD, command);

    io_out8(PIT_PORT_COUNTER0, (uint8_t)(count & 0xff));
    io_out8(PIT_PORT_COUNTER0, (uint8_t)((count >> 8) & 0xff));

    return;
}

void exception_handler(trap_frame* t_frame)
{
    printk("t_frame address 0x%x", t_frame);
    printk("exception occured: %s.", error_name[t_frame->trap_number]);
    printk("program counter is 0x%x", t_frame->eip);
    printk("eflags is 0x%x", t_frame->eflags);

    printk("error_code %d", t_frame->error_code);
    printk("cs %d\n", t_frame->cs);
    while (true) {
        io_hlt();
    }
}

void irq_handler(IRQ_HANDLER_NUM irq)
{
    printk("irq_handler : %d", irq);
    switch (irq) {
        case IRQ_CLOCK:
            timer_interrupt_handler();
            break;

        case IRQ_KEYBOARD: {
/*             keyboard_interrupt(); */
            break;
        }
        default:
            break;
    }
    return;
}

void system_call_handler(system_call_number num)
{
    printk("in system call handler %d", num);
    return;
}

void timer_interrupt_handler(void)
{
    schedule();
}

