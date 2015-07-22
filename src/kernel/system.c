#include "system.h"
#include "kernel.h"
#include "asm.h"

#include <stdbool.h>

void system(void)
{
    printk("start system kernel thread");

    io_sti();

    initalize();

/*     test_count(0); */
    while (true) {
        io_hlt();
    }
}

void initalize(void)
{
}

bool test_count(int a)
{
    return false;
}
