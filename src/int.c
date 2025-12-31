#include "x86_int.h"
#include "x86_int_def.h"
#include <hm/exception.h>
#include <hm/int.h>
#include <hm/print.h>
#include <hm/string.h>
#include <stdint.h>

/*
 * - IDT Entry format
 *    31                                      0
 * 12| reserved                                |
 *    31                                      0
 * 8 | offset(63..32)                          |
 *    31              16 . ..  . 11 8 7 3 2   0
 * 4 | offset(31..16)   |P|DPL|0|TYPE| 0 | IST |
 *    31              16 15                   0
 * 0 | segment selector | offset(15..0)        |
 */

#define IDT_ENTRY_FLAG_OFFSET_IST (0)
#define IDT_ENTRY_FLAG_MASK_IST (0b111 << IDT_ENTRY_FLAG_OFFSET_IST)
#define IDT_ENTRY_FLAG_IST_INT_GATE (0xe << IDT_ENTRY_FLAG_OFFSET_IST)
#define IDT_ENTRY_FLAG_IST_TRAP_GATE (0xf << IDT_ENTRY_FLAG_OFFSET_IST)
#define IDT_ENTRY_FLAG_OFFSET_TYPE (8)
#define IDT_ENTRY_FLAG_MASK_TYPE (0b1111 << IDT_ENTRY_FLAG_OFFSET_TYPE)
#define IDT_ENTRY_FLAG_OFFSET_DPL 13
#define IDT_ENTRY_FLAG_MASK_DPL (0b11 << IDT_ENTRY_FLAG_OFFSET_DPL)
#define IDT_ENTRY_FLAG_DPL_
#define IDT_ENTRY_FLAG_OFFSET_P 15
#define IDT_ENTRY_FLAG_MASK_P (0b1 << IDT_ENTRY_FLAG_OFFSET_P)
#define IDT_ENTRY_FLAG_PRESENT (0b1 << IDT_ENTRY_FLAG_OFFSET_P)

struct idt_entry {
  uint16_t offset_0_15;
  uint16_t segment_selector;
  uint16_t flags;
  uint16_t offset_31_16;
  uint32_t offset_63_32;
  uint32_t reserved;
} __attribute__((packed));

struct idtr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

#define IDT_MAX_ENTRY 256
static struct idt_entry idt[IDT_MAX_ENTRY];

static inline void load_idt(struct idtr *idtr) {
  asm volatile("lidt (%0)" : : "r"(idtr) : "memory");
}

static int fill_idt_entry(int ec_num, void (*asm_irq_handler)(void)) {
  uint64_t addr = (uint64_t)asm_irq_handler;

  if (ec_num >= IDT_MAX_ENTRY)
    return -1;

  struct idt_entry *entry = &idt[ec_num];
  *entry = (struct idt_entry){
      .offset_0_15 = addr & 0xffff,
      .offset_31_16 = (addr >> 16) & 0xffff,
      .offset_63_32 = addr >> 32 & 0xffffffff,
      .segment_selector = 0x8,
      .flags = 0x8e00, // P=1 (bit 15), DPL=0 (bits 13-14), Type=0xE (bits
                       // 8-11), IST=0 (bits 0-2)
  };

  return 0;
}

struct irq_handler_with_ctx {
  irq_handler_t handler;
  void *ctx;
};

// static irq_handler_t irq_handlers[256];
static struct irq_handler_with_ctx irq_handlers[256];
static uint64_t irq_handler_bitmap[4]; // 64 * 4 == 256

static void reset_irq_handler_bitmap(void) {
  memset(irq_handler_bitmap, 0x00, sizeof(uint64_t) * 4);
}

static int find_first_empty(void) {
  for (int i = 0; i < 4; i++) {
    uint64_t tmp = irq_handler_bitmap[i];
    if (!tmp)
      continue;

    uint64_t mask = 1;
    for (int j = 0; j < 64; j++, mask <<= 1) {

      if (tmp & mask)
        continue;

      return 64 * i + j;
    }
  }

  return -1;
}

int set_irq_handler(uint64_t irqn, irq_handler_t irq_handler, void *ctx) {
  int idx = irqn / 64;
  int offset = irqn % 64;
  uint64_t tmp = irq_handler_bitmap[idx];

  if (tmp & (1ULL << offset))
    kprintf("overwrite the interrupt hanlder for %d\n", irqn);

  tmp |= 1ULL << offset;

  irq_handler_bitmap[idx] = tmp;

  irq_handlers[irqn] = (struct irq_handler_with_ctx){
      .handler = irq_handler,
      .ctx = ctx,
  };

  return 0;
}

int register_irq_handler(irq_handler_t irq_handler, uint16_t *irqn, void *ctx) {

  int idx = find_first_empty();
  if (idx < 0)
    return -1;

  int ret = set_irq_handler(idx, irq_handler, ctx);
  if (ret)
    return -1;

  *irqn = idx; // Set the allocated IRQ number
  return 0;
}

int int_init(void) {

  memset(idt, 0x00, sizeof(idt));

  struct idtr idtr = {
      .limit = sizeof(idt) - 1, // Size in bytes minus 1
      .base = (uint64_t)idt,
  };

  fill_idt_entry(EXC_NUM_DIVIDE_ERROR, irq_handler_0);
  fill_idt_entry(EXC_NUM_DEBUG, irq_handler_1);
  fill_idt_entry(EXC_NUM_NMI, irq_handler_2);
  fill_idt_entry(EXC_NUM_BREAKPOINT, irq_handler_3);
  fill_idt_entry(EXC_NUM_OVERFLOW, irq_handler_4);
  fill_idt_entry(EXC_NUM_BOUND_RANGE_EXCEEDED, irq_handler_5);
  fill_idt_entry(EXC_NUM_INVALID_OPCODE, irq_handler_6);
  fill_idt_entry(EXC_NUM_DEVICE_NOT_AVAIL, irq_handler_7);
  fill_idt_entry(EXC_NUM_DOUBLE_FAULT, irq_handler_8);
  fill_idt_entry(EXC_NUM_COPROC_SEGMENT_OVERRUN, irq_handler_9);
  fill_idt_entry(EXC_NUM_INVALID_TSS, irq_handler_10);
  fill_idt_entry(EXC_NUM_SEGMENT_NOT_PRESENT, irq_handler_11);
  fill_idt_entry(EXC_NUM_STACK_FAULT, irq_handler_12);
  fill_idt_entry(EXC_NUM_GENERAL_PROTECTION, irq_handler_13);
  fill_idt_entry(EXC_NUM_PAGE_FAULT, irq_handler_14);
  fill_idt_entry(EXC_NUM_FPU_ERROR, irq_handler_16);
  fill_idt_entry(EXC_NUM_ALIGN_CHECK, irq_handler_16);
  fill_idt_entry(EXC_NUM_MACHINE_CHECK, irq_handler_17);
  fill_idt_entry(EXC_NUM_SIMD_FP, irq_handler_19);
  fill_idt_entry(EXC_NUM_VIRT, irq_handler_20);
  fill_idt_entry(EXC_NUM_CONTROL_PROTECT, irq_handler_21);

  fill_idt_entry(32, irq_handler_32);
  fill_idt_entry(33, irq_handler_33);
  fill_idt_entry(34, irq_handler_34);
  fill_idt_entry(35, irq_handler_35);
  fill_idt_entry(36, irq_handler_36);

  fill_idt_entry(255, irq_handler_255); // Spurious interrupt vector

  reset_irq_handler_bitmap();
  set_exception_handlers();

  // Fill all remaining IDT entries with default handler to prevent triple fault
  for (int i = 0; i < IDT_MAX_ENTRY; i++) {
    if (idt[i].offset_0_15 == 0 && idt[i].offset_31_16 == 0 &&
        idt[i].offset_63_32 == 0) {
      fill_idt_entry(i, irq_handler_255);
    }
  }

  load_idt(&idtr);

  return 0;
}

void irq_handler(struct context *context) {

  // Debug: print all non-timer interrupts
  if (context->reason != 255)
    kprintf("IRQ: vector=%d\n", context->reason);

  irq_handlers[context->reason].handler(context->reason, context,
                                        irq_handlers[context->reason].ctx);

  return;
}
