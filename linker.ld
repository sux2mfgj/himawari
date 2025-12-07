OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_boot)

PHDRS {
  text PT_LOAD FLAGS(5);          /* R-E */
  data PT_LOAD FLAGS(6);          /* RW- */
  note PT_NOTE FLAGS(4);          /* R-- */
}

SECTIONS {

  /* AP entry */
  . = 0x10000;
  _ap_start = .;
  .text : {
    *(.text.boot16)
  } :text
  . = ALIGN(4096);
  _ap_end = .;

  . = 1M;
  _kernel_start = .;

  .note.Xen : {
    *(.note.Xen)
  } :text :note

  .text : {
    *(.text)
  } :text

  .rodata : {
    *(.rodata)
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
