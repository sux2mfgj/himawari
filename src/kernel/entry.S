#define ASM_FILE
#include <descriptor.h>

.text
.code64

#define PUSH_ALL \
    push %r15; \
    push %r14; \
    push %r13; \
    push %r12; \
    push %r11; \
    push %r10; \
    push %r9; \
    push %r8; \
    push %rdi; \
    push %rsi; \
    push %rbp; \
    push %rdx; \
    push %rcx; \
    push %rbx; \
    push %rax 

#define POP_ALL \
    pop %rax; \
    pop %rbx; \
    pop %rcx; \
    pop %rdx; \
    pop %rbp; \
    pop %rsi; \
    pop %rdi; \
    pop %r8;    \
    pop %r9;    \
    pop %r10;   \
    pop %r11; \
    pop %r12; \
    pop %r13; \
    pop %r14; \
    pop %r15


.globl trap_entry

trap_entry:
    PUSH_ALL 
    
    mov %rsp, %rdi
    call trap

trap_exit:
    POP_ALL
        # ignore error code and trap number
    add $16, %rsp
    iretq

#define define_exception_not_error_code(name, number) \
        .text;\
        .globl name;\
        name:;\
            push $0;\
            push $number;\
            jmp trap_entry


#define define_exception_error_code(name, number) \
        .text;\
        .globl name;\
            name:;\
            push $number;\
            jmp trap_entry

#define define_irq_hander(name, number) \
        .text;\
        .globl name;\
        name:;\
            push $0;\
            push $number;\
            jmp trap_entry

#define define_interrupt_hander(name, number) define_exception_not_error_code(name, number)

//.data 
//.globl vectors
//vectors:

define_exception_not_error_code(divide_error_exception, 0)
define_exception_not_error_code(debug_exception, 1)
define_exception_not_error_code(nmi_interrupt, 2)
define_exception_not_error_code(breakpoint_exception, 3)
define_exception_not_error_code(overflow_exception, 4)
define_exception_not_error_code(bound_range_exceeded_exception, 5)
define_exception_not_error_code(invalid_opecode_exception, 6)
define_exception_not_error_code(device_not_availabe_exception, 7)
define_exception_error_code(double_fault_exceptin, 8)
define_exception_not_error_code(coprocessor_segment_overrun, 9)
define_exception_error_code(invalid_tss_exception, 10)
define_exception_error_code(segment_not_present, 11)
define_exception_error_code(stack_fault_exception, 12)
define_exception_error_code(general_protection_exception, 13)
define_exception_error_code(page_fault_exception, 14)
define_exception_not_error_code(not_set_15, 15)
define_exception_not_error_code(fpu_floating_point_error, 16)
define_exception_error_code(alignment_check_exception, 17)
define_exception_not_error_code(machine_check_exception, 18)
define_exception_not_error_code(simd_floating_point_exception, 19)
define_exception_not_error_code(virtualization_exception, 20)

define_irq_hander(timer_handler, IDT_ENTRY_PIC_TIMER)
define_interrupt_hander(task_call_handler, IDT_ENTRY_TASK_CALL)

#include <segment.h>
.globl user_entry
user_entry:
    movl $USER_DATA_SEGMENT, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %gs
    movl %eax, %fs
1: 
    jmp 1b



