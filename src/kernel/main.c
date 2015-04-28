#include "kernel.h"
#include "asm.h"

void kernel_entry(uint32_t magic, MULTIBOOT_INFO *multiboot_info)
{
    io_hlt();
    return;
}
