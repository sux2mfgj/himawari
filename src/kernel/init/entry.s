.text

.globl entry_64

entry_64:
    call start_kernel
    hlt
