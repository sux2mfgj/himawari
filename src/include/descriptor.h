#pragma once

#include <stdint.h>
#include <segment.h>
#include <page.h>

enum {
    GATE_INTERRUPT = 0xE,
    GATE_TRAP = 0xF,
    GATE_CALL = 0xC,
};

#define IDT_ENTRY_NUM 256
#define VECTOR_ENTRY_NUM 256

// struct gate_descriptor {
//     uint16_t offset_low;
//     uint16_t segment_selector;
//     unsigned ist    : 3;
//     unsigned zero5  : 5;
//     unsigned type   : 4;
//     unsigned zero1  : 1;
//     unsigned dpl    : 2;
//     unsigned segment_present : 1;
//     uint16_t offset_middle;
//     uint32_t offset_high;
//     uint32_t reserved;
// }__attribute__((packed));

#define IO_BITMAP_BITS 65536
#define IO_BITMAP_BYTES (IO_BITMAP_BITS / 8)
#define IO_BITMAP_LONGS (IO_BITMAP_BYTES / sizeof(long))

struct desc_struct {
    uint16_t limit0;
    uint16_t base0;

    unsigned base1 : 8, type : 4, s : 1, dpl : 2, p : 1;
    unsigned limit : 4, avl : 1, l : 1, d : 1, g : 1, base2 : 8;
};

extern struct desc_struct gdt_table[];

struct tss_struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint32_t reserved2;
    uint32_t reserved3;
    uint16_t reserved4;
    uint16_t io_map_base_addr;
    unsigned long io_bitmap[IO_BITMAP_LONGS + 1];
} __attribute__((packed));

enum {
    DESC_TSS = 0x9,
    DESC_LDT = 0x2,
};

struct ldttss_desc {
    uint16_t limit0;
    uint16_t base0;
    unsigned base1 : 8, type : 5, dpl : 2, p : 1;
    unsigned limit1 : 4, zero0 : 3, g : 1, base2 : 8;
    uint32_t base3;
    uint32_t zero1;
} __attribute__((packed));

static inline void set_tssldt_descriptor(uintptr_t ptr,
                                         uintptr_t tss,
                                         unsigned type,
                                         unsigned size)
{
    struct ldttss_desc* d = (struct ldttss_desc*)ptr;
    d->limit0 = size & 0xFFFF;
    d->base0 = tss & 0xFFFF;
    d->base1 = ((tss >> 16) & 0xFFFF) & 0xFF;
    d->type = type;
    d->p = 1;
    d->limit1 = (size >> 16) & 0xF;
    d->base2 = (((tss >> 16) & 0xFFFF) >> 8) & 0xFF;
    d->base3 = (tss >> 32);
}

static inline void set_tss_desc(uintptr_t addr)
{
    set_tssldt_descriptor(&gdt_table[TASK_STATE_SEGMENT >> 3], addr, DESC_TSS,
                          sizeof(struct tss_struct) - 1);
}

struct gate_descriptor {
    uint16_t offset_low;
    uint16_t segment;
    unsigned ist : 3, zero0 : 5, type : 5, dpl : 2, p : 1;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t zero1;
} __attribute__((packed));

struct descriptor_ptr {
    uint16_t size;
    uint64_t addr;
} __attribute__((packed));

static inline void _set_gate(struct gate_descriptor* adr,
                             unsigned type,
                             uintptr_t func,
                             unsigned dpl,
                             unsigned ist)
{
    adr->offset_low = (uint16_t)(func & 0xFFFF);
    adr->segment = KERNEL_CODE_SEGMENT;
    adr->ist = ist;
    adr->p = 1;
    adr->dpl = dpl;
    adr->zero0 = 0;
    adr->zero1 = 0;
    adr->type = type;
    adr->offset_middle = (func >> 16) & 0xFFFF;
    adr->offset_high = (func >> 32);
}

extern struct gate_descriptor idt_table[];

static inline void set_intr_gate(int nr, void* func)
{
    _set_gate((struct gate_descriptor*)((uintptr_t)&idt_table[nr] +
                                        (uintptr_t)START_KERNEL_MAP),
              GATE_INTERRUPT, (uintptr_t)func, 0, 0);
}

extern void divide_error_exception(void);
extern void debug_exception(void);
extern void nmi_interrupt(void);
extern void breakpoint_exception(void);
extern void overflow_exception(void);
extern void bound_range_exceeded_exception(void);
extern void invalid_opecode_exception(void);
extern void device_not_availabe_exception(void);
extern void double_fault_exceptin(void);
extern void coprocessor_segment_overrun(void);
extern void invalid_tss_exception(void);
extern void segment_not_present(void);
extern void stack_fault_exception(void);
extern void general_protection_exception(void);
extern void page_fault_exception(void);
extern void not_set_15(void);
extern void fpu_floating_point_error(void);
extern void alignment_check_exception(void);
extern void machine_check_exception(void);
extern void simd_floating_point_exception(void);
extern void virtualization_exception(void);

enum {
    // 32 ~ 255 is availabel
    // IDT_ENTRY_SYSTEM_CALL = 32,
    IDT_ENTRY_TIMER = 32,
};
