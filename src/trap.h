#ifndef _INCLUDED_TRAP_H_
#define _INCLUDED_TRAP_H_

#include "vectors.h"
#include "segment.h"
#include "keyboard.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define INTERRUPT_DESCRIPTOR_TABLE_SIZE 256

#define HARDWARE_VECTOR(n) n + 32

// only 32bit protect mode
enum GATE_TYPE { TAKS_GATE = 0x5, INTERRUPT_GATE = 0xe, TRAP_GATE = 0xf };

enum PRIVILEGE_LEVEL { PRIVILEGE_OS = 0, PRIVILEGE_USER = 3 };

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

static char *error_name[] = {
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

typedef enum { SYS_TEST, } SYSTEM_CALL_NUM;

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
    enum PRIVILEGE_LEVEL level;
} gate_vector_table;

void init_interrupt(void);

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
typedef struct _trap_frame {
    // registers as pushed by pusha
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // set of trap frame
    uint16_t gs;
    uint16_t padding0;
    uint16_t fs;
    uint16_t padding1;
    uint16_t es;
    uint16_t padding2;
    uint16_t ds;
    uint16_t padding3;

    //
    uint16_t trap_number;

    //defined by x86 hardware
    uint32_t error_code;
    uint32_t eip;
    uint16_t cs;
    uint16_t padding4;
    uint32_t eflags;

    // from user to kernel
//     uint32_t user_esp;
//     uint16_t ss;
//     uint16_t padding5;
}trap_frame;

void exception_handler(trap_frame*);
void irq_handler(IRQ_HANDLER_NUM irq);
void system_call_handler(SYSTEM_CALL_NUM num);

#endif
