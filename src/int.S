.intel_syntax noprefix

#include "x86_int_def.h"

.macro irqh_noerr num
.global irq_handler_\num
irq_handler_\num:
  push 0
  push \num
  jmp irqh_common
.endm

.macro irqh_err num
.global irq_handler_\num
irq_handler_\num:
  push \num
  jmp irqh_common
.endm

irqh_noerr EXC_NUM_DIVIDE_ERROR
irqh_noerr EXC_NUM_DEBUG
irqh_noerr EXC_NUM_NMI
irqh_noerr EXC_NUM_BREAKPOINT
irqh_noerr EXC_NUM_OVERFLOW
irqh_noerr EXC_NUM_BOUND_RANGE_EXCEEDED
irqh_noerr EXC_NUM_INVALID_OPCODE
irqh_noerr EXC_NUM_DEVICE_NOT_AVAIL
irqh_err EXC_NUM_DOUBLE_FAULT
irqh_noerr EXC_NUM_COPROC_SEGMENT_OVERRUN
irqh_err EXC_NUM_INVALID_TSS
irqh_err EXC_NUM_SEGMENT_NOT_PRESENT
irqh_err EXC_NUM_STACK_FAULT
irqh_err EXC_NUM_GENERAL_PROTECTION
irqh_err EXC_NUM_PAGE_FAULT
irqh_noerr EXC_NUM_FPU_ERROR
irqh_err EXC_NUM_ALIGN_CHECK
irqh_noerr EXC_NUM_MACHINE_CHECK
irqh_noerr EXC_NUM_SIMD_FP
irqh_noerr EXC_NUM_VIRT
irqh_err EXC_NUM_CONTROL_PROTECT

irqh_noerr 32
irqh_noerr 33
irqh_noerr 34
irqh_noerr 35
irqh_noerr 36

// Spurious interrupt (vector 255)
irqh_noerr 255

irqh_common:
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rbp
  push rdi
  push rsi
  push rdx
  push rcx
  push rbx
  push rax
  mov rdi, rsp
  call irq_handler
  pop rax
  pop rbx
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  pop rbp
  pop r8
  pop r9
  pop r10
  pop r11
  pop r12
  pop r13
  pop r14
  pop r15

  add rsp, 0x10

  iretq
