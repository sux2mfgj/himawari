.file "function.s"

.text
.code32

.globl io_hlt
io_hlt:
    hlt
    ret

.globl io_cli
io_cli:
    cli
    ret

.globl io_sti
io_sti:
    sti
    ret

