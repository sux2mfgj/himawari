ENTRY(entry_64)

#define KERNEL_VIRTUAL_BASE 0x0000000000100000
#define KERNEL_PHYSICAL_BASE 0x0000000000100000

SECTIONS
{
    . = KERNEL_VIRTUAL_BASE;

    .text : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
    {
        *(.text)
    }

    .data : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data)
    }
}
