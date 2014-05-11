#include"graphic.h"
#include"segment.h"
#include"func.h"
#include"multiboot.h"
#include"memory.h"
#include"lib.h"


void kernel_entry(uint32_t magic, MULTIBOOT_INFO *multiboot_info)
{

    io_cli();
    init_gdtidt();
    init_pit();
    init_pic();
    init_memory();
    io_sti();

    printf(TEXT_MODE_SCREEN_LEFT, "hello");
/*     integer_puts(multiboot_info->mem_upper, 21); */
/*     integer_puts(multiboot_info->mmap_addr, 22); */
/*     integer_puts(multiboot_info->mmap_length, 23); */
    list_test();

    for(;;){
        io_hlt();
    }
}


