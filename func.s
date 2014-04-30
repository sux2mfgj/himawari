.file "func.s"

.text
.code32

.extern inthandler21
.extern timer_interrupt

.globl io_hlt, io_cli, io_sti
.globl io_in8, io_in16, io_in32, io_out8, io_out16, io_out32
.globl io_load_eflags, io_store_eflags, write_mem8
.globl load_gdtr, load_idtr, asm_inthandler21

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


