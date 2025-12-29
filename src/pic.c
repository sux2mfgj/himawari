#include <hm/io.h>
#include <hm/pic.h>
#include <hm/print.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

void pic_disable(void) {
  // Mask all interrupts on both PICs
  outb(PIC1_DATA, 0xFF);
  io_wait();
  outb(PIC2_DATA, 0xFF);
  io_wait();

  kprintf("8259 PIC disabled\n");
}
