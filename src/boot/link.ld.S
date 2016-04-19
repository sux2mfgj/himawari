ENTRY(entry)

#define ASM_FILE
#include <page.h>

SECTIONS
{

    . = PHYSICAL_START;
    .boot :
    {
        *(.boot)
    }

    . += START_KERNEL_MAP;
    _kernel_start = .;
    .text : AT(ADDR(.text) - START_KERNEL_MAP)
    {
        *(.text)
    }

    .data : AT(ADDR(.data) - START_KERNEL_MAP)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - START_KERNEL_MAP)
    {
        *(.bss)
    }

    _kernel_end = .;
}

