.text

.globl entry_64

entry_64:
    hlt
    hlt
    hlt

    call start_kernel

    hlt
