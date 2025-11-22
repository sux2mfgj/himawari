OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_boot)

PHDRS {
  text PT_LOAD FLAGS(5);          /* R-E */
  data PT_LOAD FLAGS(6);          /* RW- */
  note PT_NOTE FLAGS(4);          /* R-- */
}

SECTIONS {
  . = 1M;

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
  } :data

  . = ALIGN(4096);
  __early_stack_bottom = .;
  . += 0x2000;
  __early_stack_top = .;
}
