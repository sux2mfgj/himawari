ENTRY(entry)
OUTPUT_FORMAT(elf32-i386)

physical_base = 0x00100000;
virtual_base = 0xc0000000;

SECTIONS
{
    . = physical_base + virtual_base;
    _kernel_start = .;


    .text : AT( ADDR(.text) - virtual_base){
        _text_start = .;
        *(.entry)
        *(.text)
        _text_end = .;
    }
    .data : AT( ADDR(.data) - virtual_base){
        _data_start = .;
        *(.data)
        _data_end = .;
    }
    .bss ALIGN(4): AT( ADDR(.bss) - virtual_base){
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }
    .rodata : AT(ADDR(.rodata) - virtual_base){
        _rodata_start = .;
        *(.rodata)
        _rodata_end = .;
    }

    _device_driver_start = .;
    _device_driver_end = .;

    _kernel_end = .;
}
