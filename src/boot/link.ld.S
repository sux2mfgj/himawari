ENTRY(entry)
//ENTRY(phy_entry)

#define ASM_FILE
#include <page.h>

SECTIONS
{
//    . = PHYSICAL_START + START_KERNEL_MAP;


    . = PHYSICAL_START;
//    .boot : AT(ADDR(.boot) - START_KERNEL_MAP)
    .boot :
    {
        //*(.bcode)
        //*(.bdata)
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

