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

