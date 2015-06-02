#include "kernel.h"
#include "asm.h"
#include "segment.h"
#include "graphic.h"
#include "trap.h"
#include "kvmemory.h"

void kernel_entry(const uint32_t magic, const multiboot_info *mb_info)
{
    io_cli();

    if(!init_text_screen()){
        //kernel panic
        kernel_panic("init_text_screen");
    }

    if(!init_gdt()){
        kernel_panic("init_gdt");
    }

    if(!init_interrupt()){
        kernel_panic("init_interrupt");
    }

    if(!init_kvmemory(mb_info)){
        kernel_panic("init_kvmemory");
    }

    printk("start himawari");

    io_sti();
    while (true) {}
    return;
}

void kernel_panic_handler(const char* name, const char* file, const int line)
{
    io_hlt();
}
