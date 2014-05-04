.file "func.s"

.text
.code32

.extern inthandler21
.extern timer_interrupt

.globl io_hlt, io_cli, io_sti
.globl io_in8, io_in16, io_in32, io_out8, io_out16, io_out32
.globl io_load_eflags, io_store_eflags, write_mem8
.globl load_gdtr, load_idtr, load_cr0, store_cr0
.globl memtest_sub
.globl asm_inthandler21, asm_timer_inthandler

io_hlt:
    hlt
    ret

write_mem8:
    movl 4(%esp), %ecx
    movb 8(%esp), %al
    movb %al, (%ecx)
    ret

io_cli:
    cli
    ret

io_sti:
    sti
    ret


io_in8:
    movl 4(%esp), %edx
    movl $0, %eax
    inb %dx, %al
    ret

io_in16:
    movl 4(%esp), %edx
    movl $0, %eax
    inw %dx, %ax
    ret

io_in32:
    movl 4(%esp), %edx
    inl %dx, %eax
    ret

io_out8:
    movl 4(%esp), %edx
    movb 8(%esp), %al
    outb %al, %dx
    ret

io_out16:
    movl 4(%esp), %edx
    movl 8(%esp), %eax
    outw %ax, %dx
    ret

io_out32:
    movl 4(%esp), %edx
    movl 8(%esp), %eax
    outl %eax, %dx
    ret

io_load_eflags:
    pushf
    pop %eax
    ret

io_store_eflags:
    movl 4(%esp), %eax
    push %eax
    popf
    ret

load_gdtr:
    movw 4(%esp), %ax
    movw %ax, 6(%esp)
    lgdt 6(%esp)
    ret

load_idtr:
    movw 4(%esp), %ax
    movw %ax, 6(%esp)
    lidt 6(%esp)
    ret

load_cr0:
    movl %cr0, %eax
    ret

store_cr0:
    movl 4(%esp), %eax
    movl %eax, %cr0
    ret

memtest_sub:
    push %edi
    push %esi
    push %ebx
    movl $0xaa55aa55, %esi
    movl $0x55aa55aa, %edi
    movl 16(%esp), %eax
mts_loop:
    movl %eax, %ebx
    addl $0x00000ffc, %ebx
    movl (%ebx), %edx
    movl %esi, (%ebx)
    xorl $0xffffffff, (%ebx)
    cmpl (%ebx), %edi
    jne mts_fin
    xorl $0xffffffff, (%ebx)
    cmpl (%ebx), %esi
    jne mts_fin
    movl %edx, (%ebx)
    addl $0x00001000, %eax
    cmp 20(%esp), %eax
    jbe mts_loop
    popl %ebx
    popl %esi
    popl %edi
    ret

mts_fin:
    movl %edx, (%ebx)
    popl %ebx
    popl %esi
    popl %edi
    ret


asm_inthandler21:
    pushw %es
    pushw %ds
    pusha
    movl %esp, %eax
    pushl %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler21
    popl %eax
    popa
    popw %ds
    popw %es
    iret

asm_timer_inthandler:
     pushw %es
    pushw %ds
    pusha
    movl %esp, %eax
    pushl %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call timer_interrupt
    popl %eax
    popa
    popw %ds
    popw %es
    iret


