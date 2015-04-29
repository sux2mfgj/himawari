#include "kernel.h"
#include "asm.h"
#include "graphic.h"

void kernel_entry(const uint32_t magic, const MULTIBOOT_INFO *multiboot_info)
{
    io_cli();

    if(!init_screen()){
        //kernel panic
        io_hlt();
    }


    io_hlt();
    return;
}
