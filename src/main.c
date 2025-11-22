#include <hm/print.h>
#include <pvh.h>

void qemu_debugcon_putc(char c);

void kernel_cmain(struct hvm_start_info *start_info) {
  register_putc(qemu_debugcon_putc);
  kprintf("hello world\n");

  kprintf("memmap_paddr 0x%x\n", start_info->memmap_paddr);

  asm volatile("hlt");
}
