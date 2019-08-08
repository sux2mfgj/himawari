.text

.globl entry_64

entry_64:
    movq (%rdi), %rsp
    call start_kernel
    hlt
