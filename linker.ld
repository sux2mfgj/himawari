OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_boot)

PHDRS {
  text PT_LOAD FLAGS(5);          /* R-E */
  data PT_LOAD FLAGS(6);          /* RW- */
  note PT_NOTE FLAGS(4);          /* R-- */
}

SECTIONS {

  /* Reserve space for AP boot code at 0x10000 */
  _ap_start = 0x10000;
  _ap_end = 0x11000;

  . = 1M;
  _kernel_start = .;

  .note.Xen : {
    *(.note.Xen)
  } :text :note

  .text : {
    *(.text)
    . = ALIGN(4096);
    _ap_boot_code_start = .;
    *(.text.boot16)
    . = ALIGN(4096);
    _ap_boot_code_end = .;
  } :text

  .rodata : {
    *(.rodata)

    /* Module init function pointers */
    . = ALIGN(8);
    __module_init_start = .;
    *(.module_init)
    __module_init_end = .;
  } :text

  .data : {
    *(.data)
  } :data

  .bss : {
    *(.bss)

    . = ALIGN(4096);
    _early_stack_bottom = .;
    . += 0x2000;
    _early_stack_top = .;
    . += 0x1000;
    . = ALIGN(0x1000);

    _early_heap_start = .;
    . += 0x1000 * 10;
    _early_heap_end = .;
  } :data
  _kernel_end = .;
}
