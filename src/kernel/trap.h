#ifndef _INCLUDED_TRAP_H_
#define _INCLUDED_TRAP_H_

// #include "vectors.h"
// #include "segment.h"
// #include "keyboard.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// #define INTERRUPT_DESCRIPTOR_TABLE_SIZE 256

#define HARDWARE_VECTOR(n) n + 32

// only 32bit protect mode
typedef enum { TAKS_GATE = 0x5, INTERRUPT_GATE = 0xe, TRAP_GATE = 0xf }gate_type;

typedef enum { PRIVILEGE_OS = 0, PRIVILEGE_USER = 3 }privilege_level;

typedef enum {
    // master
    IRQ_CLOCK,
    IRQ_KEYBOARD,
    IRQ_CASCADE_SLAVE,
    IRQ_SECOND_SERIAL,
    IRQ_FIRST_SERIAL,
    IRQ_XT_WINCHESTER,
    IRQ_FLOPPY,
    IRQ_PRINTER
    // slave
} IRQ_HANDLER_NUM;

static const char *error_name[] = {
    "divide by zero error",
    "debug",
    "non-maskable interrupt",
    "breakpoint",
    "overflow",
    "bound range exceeded",
    "invalid opecode",
    "device not available",
    "double fault",
    "coprocessor segment overrun",
    "invalid TSS",
    "Segment Not Presetnt",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page fault",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
};

typedef enum { SYS_TEST, }system_call_number;

typedef struct _gate_discriptor {
    uint16_t offset_low;
    uint16_t selector;

    uint8_t unused;

    unsigned gate_type : 4;
    unsigned storage_segment : 1;
    unsigned descriptor_privilege_level : 2;
    unsigned present : 1;

    uint16_t offset_high;
} gate_descriptor;

typedef struct _gate_vector_table {
    void (*gate)(void);
    uint32_t vector_num;
    privilege_level level;
} gate_vector_table;

bool ikenit_interrupt(void);

static inline void _set_gatedesc(gate_descriptor *gd, uint32_t offset,
                                 uint32_t selector, uint8_t gate_type,
                                 uint8_t storage_segment,
                                 uint8_t descriptor_privilege_level,
                                 uint8_t present);

// pic asmblly handlers
extern void hardware_interrupt0(void);
extern void hardware_interrupt1(void);
extern void hardware_interrupt2(void);
extern void hardware_interrupt3(void);
extern void hardware_interrupt4(void);
extern void hardware_interrupt5(void);
extern void hardware_interrupt6(void);
extern void hardware_interrupt7(void);
extern void hardware_interrupt8(void);
extern void hardware_interrupt9(void);
extern void hardware_interrupt10(void);
extern void hardware_interrupt11(void);
extern void hardware_interrupt12(void);
extern void hardware_interrupt13(void);
extern void hardware_interrupt14(void);
extern void hardware_interrupt15(void);

// exception asmblly handlers
extern void divide_error(void);
extern void debug_fault(void);
extern void nmi_interrupt(void);
extern void break_point(void);
extern void over_flow(void);
extern void br_bound(void);
extern void invalid_op(void);
extern void device_not(void);
extern void double_fault(void);
extern void coproc_segment_overrun(void);
extern void invalid_tss(void);
extern void segment_not_fault(void);
extern void stack_exception(void);
extern void general_protection(void);
extern void page_fault(void);
extern void coproc_err(void);
extern void alignment_check(void);
extern void machine_check(void);
extern void simd_exception(void);

// system call assmblly handler
extern void system_call(void);

// trap frame
typedef struct{
//     registers as pushed by pusha
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

//     set of trap frame
    uint16_t gs;
    uint16_t padding0;
    uint16_t fs;
    uint16_t padding1;
    uint16_t es;
    uint16_t padding2;
    uint16_t ds;
    uint16_t padding3;


    uint16_t trap_number;

//     defined by x86 hardware
    uint32_t error_code;
    uint32_t eip;
    uint16_t cs;
    uint16_t padding4;
    uint32_t eflags;

//     from user to kernel
//     uint32_t user_esp;
//     uint16_t ss;
//     uint16_t padding5;
}trap_frame;

void exception_handler(trap_frame*);
void irq_handler(IRQ_HANDLER_NUM irq);
void system_call_handler(system_call_number num);

//PIT
#define PIT_PORT_COUNTER0       0x40
#define PIT_PORT_COUNTER1       0x41
#define PIT_PORT_COUNTER2       0x41
#define PIT_PORT_CONTROL_WORD   0x43

#define PIT_CONTROL_WORD_BCD_BINARY 0
#define PIT_CONTROL_WORD_BCD_BCD    1

#define PIT_CONTROL_WORD_MODE_TIMER         0x00
#define PIT_CONTROL_WORD_MODE_ONESHOT_TIMER 0x01
#define PIT_CONTROL_WORD_MODE_PULSE         0x02
#define PIT_CONTROL_WORD_MODE_SQARE         0x03
#define PIT_CONTROL_WORD_MODE_SOFTWARE      0x04
#define PIT_CONTROL_WORD_MODE_HARDWARE      0x05

#define PIT_CONTROL_WORD_RL_LOAD    0x0
#define PIT_CONTROL_WORD_RL_LSB     0x1  // write: LSB( Least Significant Byte )
#define PIT_CONTROL_WORD_RL_MSB     0x2  // write: MSB( Most Significant Byte )
#define PIT_CONTROL_WORD_RL_LSB_MSB 0x3  // write: LSB and MSB

#define PIT_CONTROL_WORD_SC_COUNTER0    0x00
#define PIT_CONTROL_WORD_SC_COUNTER1    0x01
#define PIT_CONTROL_WORD_SC_COUNTER2    0x02
#define PIT_CONTROL_WORD_SC_DISABLE     0x03

#define PIT_CH0_CLK     1193181.67
#define PIT_CLK_1MS     PIT_CH0_CLK / 1000
#define PIT_CLK_10MS    PIT_CH0_CLK / 100

//PIC
#define PIC_MASTER_CMD_STATE_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMD_STATE_PORT 0xA0
#define PIC_SLAVE_DATA_PORT 0xA1

#define PIC_MASTER_ICW1 0x11
#define PIC_MASTER_ICW2 0x20 // use after 0x20 number interrupt descriptor table
#define PIC_MASTER_ICW3 0x04
// #define PIC_MASTER_ICW4 0x01
#define PIC_MASTER_ICW4 0x00

#define PIC_SLAVE_ICW1 PIC_MASTER_ICW1
#define PIC_SLAVE_ICW2 0x28
#define PIC_SLAVE_ICW3 0x02
#define PIC_SLAVE_ICW4 PIC_MASTER_ICW4

#define PIC_IMR_MASK_IRQ0 0x01
#define PIC_IMR_MASK_IRQ1 0x02
#define PIC_IMR_MASK_IRQ2 0x04
#define PIC_IMR_MASK_IRQ3 0x08
#define PIC_IMR_MASK_IRQ4 0x10
#define PIC_IMR_MASK_IRQ5 0x20
#define PIC_IMR_MASK_IRQ6 0x40
#define PIC_IMR_MASK_IRQ7 0x80
#define PIC_IMR_MASK_IRQ_ALL 0xff

#define PIC_OCW2_EOI 0x20

void init_pic(void);
void init_pit(void);
void set_pit_count(uint16_t count, uint8_t counter, uint8_t mode);

void timer_interrupt_handler(void);

#endif
