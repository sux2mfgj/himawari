#include <hm/io.h>
#include <hm/pit.h>
#include <hm/print.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND 0x43

void pit_disable(void) {
  // Set Channel 0 to mode 0 (interrupt on terminal count), one-shot
  // Command: Channel 0, lobyte/hibyte, mode 0
  outb(PIT_COMMAND, 0x30);
  io_wait();

  // Set count to 0 (will never fire)
  outb(PIT_CHANNEL0, 0x00);
  io_wait();
  outb(PIT_CHANNEL0, 0x00);
  io_wait();

  kprintf("PIT disabled\n");
}
