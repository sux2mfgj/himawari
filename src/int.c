#include "x86_int.h"
#include "x86_int_def.h"
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

  fill_idt_entry(IRQ_NUM_TIMER, irq_handler_32);
  fill_idt_entry(IRQ_VIRTIO_NET_RX, irq_handler_33);
  fill_idt_entry(IRQ_VIRTIO_NET_TX, irq_handler_33);

  fill_idt_entry(255, irq_handler_255); // Spurious interrupt vector

  load_idt(&idtr);

  return 0;
}

struct context {
  // Registers pushed by our interrupt handler
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rbp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
  uint64_t reason;
  uint64_t err_code;
  // Values automatically pushed by CPU
  uint64_t rip;
  uint64_t cs;
  uint64_t rflags;
  uint64_t rsp;
  uint64_t ss;
} __attribute__((packed));

// Simple counter to track timer interrupts without using kprintf
volatile int timer_interrupt_count = 0;
volatile uint32_t irq_handler_called_marker = 0;

// Debug: use multiple markers to track execution flow
volatile uint32_t irq_handler_entry_marker = 0;
volatile uint32_t irq_handler_exit_marker = 0;
volatile uint64_t page_fault_addr = 0;
volatile uint64_t page_fault_err_code = 0;

void irq_handler(struct context *context) {
  // Mark entry
  irq_handler_entry_marker = 0x11111111;

  // Write vector number to marker to see what interrupt occurred
  irq_handler_called_marker = context->reason;

  if (context->reason == EXC_NUM_DOUBLE_FAULT) {
    // Double fault - very serious
    irq_handler_entry_marker = 0xDEADDEAD;
    kprintf("DOUBLE FAULT! err_code=0x%x\n", context->err_code);
    asm volatile("hlt");
  }

  if (context->reason == IRQ_NUM_TIMER) {
    // Timer interrupt
    timer_interrupt_count++;
    irq_handler_entry_marker = 0x22222222;

    // Write to EOI register directly at APIC_BASE + 0x80
    *(volatile uint32_t *)0xfee00080UL = 0;

    irq_handler_exit_marker = 0x33333333;
    return;
  }

  if (context->reason == EXC_NUM_INVALID_OPCODE) {
    // Invalid Opcode (#UD) exception
    irq_handler_entry_marker = 0xBADC0DE;
    kprintf("INVALID OPCODE! rip=0x%x, cs=0x%x, rflags=0x%x\n", context->rip,
            context->cs, context->rflags);
    asm volatile("hlt");
  }

  if (context->reason == EXC_NUM_PAGE_FAULT) {
    // Page fault - save info to markers first
    asm volatile("mov %%cr2, %0" : "=r"(page_fault_addr));
    page_fault_err_code = context->err_code;
    irq_handler_entry_marker =
        0xDEADBEEF; // Mark that we got to page fault handler

    // Now try kprintf (might cause issues)
    kprintf("PAGE FAULT! addr=0x%x, err_code=0x%x, rip=0x%x\n", page_fault_addr,
            page_fault_err_code, context->rip);
    asm volatile("hlt");
  }

  if (context->reason == 255) {
    // Spurious interrupt - don't send EOI for spurious interrupts
    irq_handler_exit_marker = 0x44444444;
    return;
  }

  // Unexpected interrupt - send EOI anyway and halt
  *(volatile uint32_t *)0xfee00080UL = 0;
  kprintf("unexpected interrupt: vector=%d, rip=0x%x\n", context->reason,
          context->rip);
  asm volatile("hlt");
}
