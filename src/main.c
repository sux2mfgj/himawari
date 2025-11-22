void qemu_debugcon_putc(unsigned char c);

void puts(const char *text) {

  for (; *text; text++) {
    qemu_debugcon_putc(*text);
  }
}

void kernel_cmain(void) {
  puts("hello world\n");

  asm volatile("hlt");
}
