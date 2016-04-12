ENTRY(entry)





SECTIONS
{



    . = 0x0000000001000000;

    .boot :
    {


        *(.boot)
    }

    . += 0xffffffff80000000;
    _kernel_start = .;
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
