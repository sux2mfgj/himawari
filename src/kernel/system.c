#include "system.h"
#include "kernel.h"
#include "asm.h"

#include <stdbool.h>

void system(void)
{
    printk("start system kernel thread");

    io_sti();

    initalize();

    while (true) {
        

    }
}

void initalize(void)
{

}


