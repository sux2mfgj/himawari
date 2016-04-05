ENTRY(start_kernel)




SECTIONS
{
    . = 0x0000000000100000;

    .text : AT(ADDR(.text) - 0x0000000000100000)
    {
        *(.text)
    }

    .data : AT(ADDR(.data) - 0x0000000000100000)
    {
        *(.data)
    }
}
