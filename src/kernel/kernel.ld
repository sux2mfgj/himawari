ENTRY(entry_64)




SECTIONS
{
    . = 0x0000000000100000;

    .text : AT(ADDR(.text))
    {
        *(.text)
    }

    .data : AT(ADDR(.data))
    {
        *(.data)
    }
}
