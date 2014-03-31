.file "func.s"

.code32
.text
.align 2

.globl write_mem8
write_mem8: /*void wirite_mem(int addr, int data)*/
    movl 4(%esp), %ecx
    movb 8(%esp), %al
    mov %al, (%eax)
    ret

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

.globl io_in8 /*int io_in8(int posrt)*/
io_in9:
    movl 4(%esp), %edx
    movl 0, %eax
    in %dx, %al
    ret
     
.globl io_in16
io_in16:
    movl 4(%esp), %edx
    movl 0, %eax
    in %dx, %al
    ret

.globl io_in32
io_in32:
    movl 4(%esp), %edx
    in %dx, %eax
    ret


.globl io_out8
io_out8:
    movl 4(%esp), %edx
    movb 4(%esp), %al
    out %ax, %dx
    ret
    
.globl io_out16
io_out16:
    movl 4(%esp), %edx
    movl 8(%esp), %eax
    out %ax, %dx
    ret

.globl io_out32
io_out32:
    movl 4(%esp), %edx
    movl 8(%esp), %eax
    out %eax, %dx
    ret

.globl io_load_flags
io_load_flags:
    pushf
    pop %eax
    ret

.globl io_store_eflags
io_store_eflags:
    mov 4(%esp), %eax
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

.globl asm_inthandler21
asm_inthandler21:
    push %es
    push %ds
    pusha
    movl %esp, %eax
    push %eax
    movw %ss, %ax
    movw %ax, %ds
    movw %ax, %es
    call inthandler21
    pop %eax
    popa
    pop %ds
    pop %es
    iret

