
.text
.code64

.globl trap_entry, trap_exit

trap_entry:

    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rdi
    push %rsi
    push %rbp
    push %rdx
    push %rcx
    push %rbx
    push %rax

    mov %rsp, %rdi
    call trap
