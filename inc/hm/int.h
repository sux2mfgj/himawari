#pragma once

#include <stdint.h>

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

typedef int (*irq_handler_t)(uint16_t, struct context *);

int int_init(void);
int register_irq_handler(irq_handler_t irq_handler, uint16_t *irqn);
int set_irq_handler(uint64_t irqn, irq_handler_t irq_handler);
