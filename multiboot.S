#define __MULTIBOOT__S 1
#include "multiboot.h"

// kernel entry
.extern _kernel_entry


.text
.code32
.globl start, _start


start:
_start:
    jmp     _multi_boot_entry

.align 4

multiboot_header:
    .long   DEF_MBH_MAGIC
    .long   DEF_MBH_FLAGS
    .long   DEF_MBH_CHECKSUM

_multi_boot_entry:

    pushl   $0
    popf

#   Multiboot Infomation
    pushl   %ebx #first argument
    pushl   %eax #second argument

    call    _kernel_entry

loop_infinite:
    hlt
    jmp     loop_infinite

