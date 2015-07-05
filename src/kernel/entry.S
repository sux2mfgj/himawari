.file "entry.S"

.extern kernel_entry
# need data for boot by grub
MULTIBOOT_HEADER_MAGIC = 0x1BADB002
MULTIBOOT_HEADER_FLAGS = 0x0003
CHECKSUM = -(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_HEADER_FLAGS)

.text
.code32
.section .entry

.align 4
multiboot_header:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long CHECKSUM
    .long 0x0
    .long 0x0
    .long 0x0
    .long 0x0
    .long 0x0

#include "paging.h"

.data
.align 0x1000
page_directory:
    .long 0x00000083
    .long 0x00400083
    .rept (KERNEL_PAGE_NUM - 2)
        .long 0x00000000
    .endr

    .long 0x00000083
    .long 0x00400083
    .rept (1024 - KERNEL_PAGE_NUM - 2)
        .long 0x00000000
    .endr

.text
.globl entry
entry:

    movl $stack_start, %esp

    lea (page_directory - VIRTUAL_KERNEL_BASE_ADDR), %ecx
    movl %ecx, %cr3

    movl %cr4, %ecx
    orl $0x0000010, %ecx
    movl %ecx, %cr4

    movl %cr0, %ecx
    orl $0x80000000, %ecx
    movl %ecx, %cr0
    lea start_in_high_her_half, %ecx
    jmp *%ecx

start_in_high_her_half:

# reset eflags
    pushl $0
    popf

    pushl %ebx
    pushl %eax

#     jmp kernel_entry
    call kernel_entry

.text
.align 4

loop:
    hlt
    jmp loop

.globl start_hlt
start_hlt:
    hlt
    jmp start_hlt

.data
.align 8
stack:
    .space 4096
    .space 4096
    .space 4096
stack_start:
