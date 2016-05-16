ENTRY(entry)




SECTIONS
{

    _kernel_start = .;
    . = 0x0000000000100000;
    .boot :
    {
        *(.boot)
    }

    . += 0xffffffff80000000;
    .text : AT(ADDR(.text) - 0xffffffff80000000)
    {
        *(.text)
    }

    .data : AT(ADDR(.data) - 0xffffffff80000000)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xffffffff80000000)
    {
        *(.bss)
    }

    _kernel_end = .;
}
