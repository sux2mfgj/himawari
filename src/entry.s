.file "entry.s"

.extern kernel_entry
# need data for boot by grub
MULTIBOOT_HEADER_MAGIC = 0x1BADB002
MULTIBOOT_HEADER_FLAGS = 0x0003
CHECKSUM = -(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_HEADER_FLAGS)

.text
.code32
.section .entry

.align 4
multiboot_header:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long CHECKSUM

.globl entry
entry:

   pushl %ebx
   pushl %eax

    jmp kernel_entry

loop:
    hlt
    jmp loop

.globl start_hlt
start_hlt:
    hlt
    jmp start_hlt

