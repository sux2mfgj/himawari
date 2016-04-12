#define ASM_FILE
#include <boot/multiboot2.h>
#include <boot/boot.h>
#include <segment.h>
#include <page.h>
#include <x86_64.h>

//.section .bcode, "ax"
.section .boot, "ax"
.code32
.globl entry 

.align MULTIBOOT_HEADER_ALIGN
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC       // magic
    .long MULTIBOOT_ARCHITECTURE_I386   // architecture
    .long header_end - multiboot_header // header_length
    .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386  \
            + (header_end - multiboot_header))  // check sum
.align MULTIBOOT_HEADER_ALIGN
information_request:
    .word MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
    .word 1
    .long request_end - information_request
    .long MULTIBOOT_TAG_TYPE_MMAP
request_end:
.align MULTIBOOT_HEADER_ALIGN
end_tag:
    .word MULTIBOOT_TAG_TYPE_END
    .word 0
    .long 8
header_end:

// entry code
entry:

    // initialize stack
    movl $(stack + STACK_SIZE), %esp

    // reset EFLAGS
    pushl $0
    popf

    // save the pointer to the multiboot infomation struct ,and grub magic number
    movl $(multiboot_info), %ecx
    movl %ebx, (%ecx)

    // grub setup segment register(cs = 0x10, ds, 0x18)
    movl $KERNEL_DATA_SEGMENT, %eax
    movl %eax, %ds

    //setup gdt
    //lgdt pre_gdt - START_KERNEL_MAP
    lgdt pre_gdt_descr

    //setup for long mode
    //PAE
    xorl %eax, %eax
    btsl $5, %eax
    movl %eax, %cr4

    //setup paging
    movl $(pre_pml4), %eax
    movl %eax, %cr3

    movl $(pre_pdpt) ,%eax
    orl $0x7, %eax
    movl %eax, pre_pml4 

    movl $(pre_kern_pdpt), %eax
    orl $0x7, %eax
    movl $(pre_pml4), %ebx
    movl %eax, 8*511(%ebx)

    movl $(pre_pd), %eax
    orl $0x7, %eax
    movl %eax, pre_pdpt 

    movl $(pre_kern_pd), %eax
    orl $0x7, %eax
    movl $(pre_kern_pdpt), %ebx
    movl %eax, 8*510(%ebx)

    //EFER
    // specified by ecx
    // read efer. 
    movl $MSR_IA32_EFER, %ecx
    rdmsr

    // write efer
    btsl $IA32_EFER_LMA, %eax
    wrmsr

    xorl %eax, %eax
    btsl $31, %eax
    btsl $0, %eax

    movl %eax, %cr0

    ljmp $KERNEL_CODE_SEGMENT, $(long_mode_entry)

.code64
.extern start_kernel
long_mode_entry:

    //stack
    movq $(stack + STACK_SIZE + START_KERNEL_MAP), %rsp

    // setup segment register
    movl $KERNEL_DATA_SEGMENT, %eax
    movl %eax, %ds
    movl %eax, %ss
    movl %eax, %es

    movl $(multiboot_info), %eax
    movl 0(%eax), %edi
    //pushq %rax
//    pushq $(multiboot_info + START_KERNEL_MAP)

    movq $start_kernel, %rax
    jmp *%rax
    
loop:
    hlt
    jmp loop


.section .boot, "ax"
.code32
stack:
    .space STACK_SIZE

multiboot_info:
    .long 0x0

pre_gdt_descr:
    .word gdt_end - gdt_table
    .long gdt_table

.align 0x1000
//64bit gdt
gdt_table:
    .quad 0x0000000000000000    // null descriptor
    .quad 0x00af9a000000ffff    // kernel code segment
    .quad 0x00cf92000000ffff    // kernel data segment
    .quad 0x00cff2000000ffff    // user data segment
    .quad 0x00affa000000ffff    // user code segment
gdt_end:

.align 0x1000
pre_pml4:
    .zero PAGE_SIZE
.align 0x1000
pre_pdpt:
    .zero PAGE_SIZE
.align 0x1000
//.org 0x4000
// 2MB page * 20 = 40MB page
// PM == VM(<40MB)
pre_pd:
    .quad 0x0000000000000183
    .quad 0x0000000000200183
    .quad 0x0000000000400183
    .quad 0x0000000000600183
    .quad 0x0000000000800183
    .quad 0x0000000000a00183
    .quad 0x0000000000c00183
    .quad 0x0000000000e00183
    .quad 0x0000000001000183
    .quad 0x0000000001200183
    .quad 0x0000000001400183
    .quad 0x0000000001600183
    .quad 0x0000000001800183
    .quad 0x0000000001a00183
    .quad 0x0000000001c00183
    .quad 0x0000000001e00183
    .quad 0x0000000002000183
    .quad 0x0000000002200183
    .quad 0x0000000002400183
    .quad 0x0000000002600183
    .fill 492, 8, 0

.align 0x1000
pre_kern_pdpt:
    .zero PAGE_SIZE

.align 0x1000
pre_kern_pd:
    .quad 0x0000000000000183
    .quad 0x0000000000200183
    .quad 0x0000000000400183
    .quad 0x0000000000600183
    .quad 0x0000000000800183
    .quad 0x0000000000a00183
    .quad 0x0000000000c00183
    .quad 0x0000000000e00183
    .quad 0x0000000001000183
    .quad 0x0000000001200183
    .quad 0x0000000001400183
    .quad 0x0000000001600183
    .quad 0x0000000001800183
    .quad 0x0000000001a00183
    .quad 0x0000000001c00183
    .quad 0x0000000001e00183
    .quad 0x0000000002000183
    .quad 0x0000000002200183
    .quad 0x0000000002400183
    .quad 0x0000000002600183
    .fill 492, 8, 0
   

