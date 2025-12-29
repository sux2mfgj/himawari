#include "x86_int_def.h"
#include <hm/int.h>
#include <hm/print.h>

static int generic_exception_handler(uint16_t irqn, struct context *ctx) {
  kprintf("unexpected interrupt: vector=%d, rip=0x%x\n", ctx->reason, ctx->rip);
  asm volatile("hlt");

  return 0;
}

static int page_fault_handler(uint16_t irqn, struct context *ctx) {
  volatile uint64_t page_fault_addr = 0;
  // Page fault - save info to markers first
  asm volatile("mov %%cr2, %0" : "=r"(page_fault_addr));

  volatile uint64_t page_fault_err_code = ctx->err_code;

  // Now try kprintf (might cause issues)
  kprintf("PAGE FAULT! addr=0x%x, err_code=0x%x, rip=0x%x\n", page_fault_addr,
          page_fault_err_code, ctx->rip);
  asm volatile("cli");
  asm volatile("hlt");

  return 0;
}

static int double_fault_hanlder(uint16_t irqn, struct context *ctx) {
  // Double fault - very serious
  kprintf("DOUBLE FAULT! err_code=0x%x\n", ctx->err_code);
  asm volatile("hlt");

  return 0;
}

static int invalid_opcode_hanlder(uint16_t irqn, struct context *ctx) {
  // Invalid Opcode (#UD) exception
  kprintf("INVALID OPCODE! rip=0x%x, cs=0x%x, rflags=0x%x\n", ctx->rip, ctx->cs,
          ctx->rflags);

  asm volatile("hlt");
  return 0;
}

int set_exception_handlers(void) {

  for (int i = 0; i < 32; i++) {
    set_irq_handler(i, generic_exception_handler);
  }

  set_irq_handler(EXC_NUM_PAGE_FAULT, page_fault_handler);
  set_irq_handler(EXC_NUM_DOUBLE_FAULT, double_fault_hanlder);
  set_irq_handler(EXC_NUM_INVALID_OPCODE, invalid_opcode_hanlder);

  return 0;
}
