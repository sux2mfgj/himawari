#define DEBUGCON_PORT 0xe9

void qemu_debugcon_putc(char c) {
  asm volatile("outb %%al, %%dx" : : "a"(c), "d"(DEBUGCON_PORT) :);
}
