#include <stddef.h>
#include <stdint.h>

extern int kprintf(const char *fmt, ...);

void hexdump_mem(void *ptr, size_t len) {
    uint8_t *data = (uint8_t *)ptr;
    size_t i, j;

    for (i = 0; i < len; i += 16) {
        // Print address
        kprintf("%016lx  ", (unsigned long)ptr + i);

        // Print hex bytes
        for (j = 0; j < 16; j++) {
            if (i + j < len) {
                kprintf("%02x ", data[i + j]);
            } else {
                kprintf("   ");
            }
            if (j == 7) {
                kprintf(" ");
            }
        }

        kprintf(" |");

        // Print ASCII representation
        for (j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = data[i + j];
            if (c >= 32 && c <= 126) {
                kprintf("%c", c);
            } else {
                kprintf(".");
            }
        }

        kprintf("|\n");
    }
}
