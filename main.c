#include"graphic.h"
#include"segment.h"
#include"func.h"

void kernel_entry()
{
    io_cli();
    init_gdtidt();
    init_pic();
    io_sti();
    init_pit();

    h_puts("hello");

    for(;;){
        io_hlt();

    }
}

