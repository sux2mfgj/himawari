#include"graphic.h"
#include"segment.h"
#include"func.h"
#include"multiboot.h"

void kernel_entry(unsigned int magic, MULTIBOOT_INFO *multiboot_info)
{

    io_cli();
    init_gdtidt();
    init_pit();
    init_pic();
    io_sti();

    h_puts("hello");
/*     integer_puts(multiboot_info->mem_upper, 21); */
/*     integer_puts(multiboot_info->mmap_addr, 22); */
/*     integer_puts(multiboot_info->mmap_length, 23); */

    for(;;){
        io_hlt();
    }
}

