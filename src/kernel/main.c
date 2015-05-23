#include "kernel.h"
#include "asm.h"
#include "graphic.h"
#include "segment.h"

void kernel_entry(const uint32_t magic, const MULTIBOOT_INFO *multiboot_info)
{
    io_cli();

    if(!init_text_screen()){
        //kernel panic
        io_hlt();
    }

    if(!init_gdt()){
        io_hlt();
    }

    printk("start himawari");

    io_hlt();
    return;
}
