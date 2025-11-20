OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_boot)

SECTIONS {
  . = 1M;

  .note.Xen : {
    *(.note.Xen)
  }

  .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }

  . = ALIGN(4096);
  __early_stack_bottom = .;
  . += 0x2000;
  __early_stack_top = .;
}
