#define ASM_FILE
#include <multiboot2.h>
#include <boot.h>

.section .boot_text, "ax"
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

    // push the pointer to the multiboot infomation struct.
    pushl %ebx
    pushl %eax

//    call start_kernel

//     pushl $halt_message
//     call printf

loop:
    hlt
    jmp loop

halt_message:
    .asciz "Halted."

    .comm stack, STACK_SIZE

