#USER_PROCESS_STACK_ADDR=0x0000 7fff ffff f000
USER_PROCESS_STACK_ADDR=0x1000

.text
.globl _start
_start:
    movq $USER_PROCESS_STACK_ADDR, %rsp
    call main

1:
    jmp 1b

