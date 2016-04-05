ENTRY(entry)

SECTIONS
{
    . = 0x00100000;

    .boot_text :
    {
        *(.boot_text)
    }

}

