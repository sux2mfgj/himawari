#define ASM_FILE
#USER_PROCESS_STACK_ADDR=0x0000 7fff ffff f000
USER_PROCESS_STACK_ADDR=0x1000

#include <segment.h>

.text
.globl _start
_start:
    movq $USER_PROCESS_STACK_ADDR, %rsp
    movl $SERVER_DATA_SEGMENT, %eax
    movl %eax, %ds
    movl %eax, %ss
    movl %eax, %es
    //TODO Is fs and gs needed?
    movl %eax, %fs
    movl %eax, %gs
    call main

1:
    jmp 1b

