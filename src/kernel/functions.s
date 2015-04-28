.file "function.s"

.text
.code32

.globl io_hlt
io_hlt:
    hlt
    ret
