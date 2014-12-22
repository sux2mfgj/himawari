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

enum IRQ_HANDLER_NUM {
    IRQ_CLOCK,
    IRQ_KEYBOARD,
    IRQ_CASCADE_SLAVE,
    IRQ_SECOND_SERIAL,
    IRQ_FIRST_SERIAL,
    IRQ_XT_WINCHESTER,
    IRQ_FLOPPY,
    IRQ_PRINTER
};

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

void exception_handler(int, int);
void irq_handler(enum IRQ_HANDLER_NUM irq);

#endif
