.file "func.s"

.text
.code32

.extern timer_inthandler
.extern keyboard_inthandler
.extern fault_inthandler

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

extern void io

extern void io
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
    pushfl
    popl %eax
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


.extern __switch_to
.globl switch_to
switch_to: #void task_switch(TASK_MANAGEMENT_DATA *prev, TASK_MANAGEMENT_DATA *next)

#     pusha
    pushf
    pushl %ebp

#store
    movl 12(%esp), %ebp
    movl %esp, 0(%ebp) # prev->esp = %esp
#     movl restore, %eax
    movl $1f, 4(%ebp) #prev->eip = $restore

#load
    movl 16(%esp), %ebp
    movl 0(%ebp), %esp     # %esp = next->esp
#    pushl $1f
    pushl 4(%ebp)       #push next->eip

#   test
#     pushl %eax
#     pushl %eax
    jmp switch_to
1:
#     jmp switch_to
    popl %ebp
    popf
    popfl
#     popa
    ret
# jmp task_switch


.globl set_page_directory
set_page_directory: #void set_page_directory(uintptr_t page_directory_addr);
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret

.globl disable_paging
disable_paging:
    movl %cr4, %eax
    andl $0xffffff7f, %eax
    movl %eax, %cr4
    ret

.globl start_paging
start_paging:
    movl %cr4, %eax
    orl $0x00000080, %eax
    movl %eax, %cr4
    ret

.globl enable_paging
enable_paging: # void enable_paging();
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    ret

.globl start_4k_paging
start_4k_paging:
    movl %cr4, %eax
    andl $0xffffffef, %eax
    movl %eax, %cr4
    ret

.globl set_esp
set_esp: #void set_esp(uintptr_t *esp);
    movl 0(%esp), %ebx
    movl 4(%esp), %eax
    movl %eax, %esp
    push %ebx
    ret


# below code is test
.globl test_sys_call
test_sys_call:
    pushl %eax
    movl $10, %eax
    int $128
    pop %eax
