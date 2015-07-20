#define _ASM_FILE_
.file "function.s"

.text
.code32

.globl io_hlt
io_hlt:
    hlt
    ret

.globl io_cli
io_cli:
    cli
    ret

.globl io_sti
io_sti:
    sti
    ret

.globl io_load_eflags
io_load_eflags:
    pushfl
    popl %eax
    ret

.globl io_store_eflags
io_store_eflags:
    movl 4(%esp), %eax
    push %eax
    popf
    ret

.globl load_gdtr
load_gdtr:
    movw 4(%esp), %ax
    movw %ax, 6(%esp)
    lgdt 6(%esp)
    ret

.globl load_idtr
load_idtr:
    movw 4(%esp), %ax
    movw %ax, 6(%esp)
    lidt 6(%esp)
    ret

.globl io_in8
io_in8:
    movl 4(%esp), %edx
    movl $0, %eax
    inb %dx, %al
    ret

.globl io_out8
io_out8:
    movl 4(%esp), %edx
    movb 8(%esp), %al
    outb %al, %dx
    ret

.extern system
.globl start_other_process
start_other_process:
    movl $system_stack_start, %esp
    movl $system_stack_start, %ebp
    jmp system

.data
.align 8
system_stack:
    .space 4096
system_stack_start:

.data
.align 8
.globl test_stack, test_stack_start
test_stack:
    .space 4096
test_stack_start:
