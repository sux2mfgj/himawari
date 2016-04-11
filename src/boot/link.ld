ENTRY(entry)





SECTIONS
{



    . = 0x0000000001000000;

    .boot :
    {


        *(.boot)
    }

    . += 0xffffffff80000000;
    .text : AT(ADDR(.text) - 0xffffffff80000000)
    {
        *(.text)
    }
}
