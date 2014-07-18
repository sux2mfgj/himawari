#include"graphic.h"
#include"segment.h"
#include"func.h"
#include"multiboot.h"
#include"memory.h"
#include"lib.h"
#include "interrupt_handler.h"
#include "task.h"


uint8_t stack[3][1024];

void task1(void)
{
    int i = 0;
/*     io_sti(); */
/*     printf(TEXT_MODE_SCREEN_RIGHT, "0x%x", io_load_eflags()); */
    for (i = 0; i < 100000; i++) {
        printf(TEXT_MODE_SCREEN_LEFT, "task1: %d", i);
    }

    while(true){
        io_hlt();
    }
}

void task2(void)
{
    int i = 0;
/*     io_sti(); */
    for (i = 0; i < 100000; i++) {
        printf(TEXT_MODE_SCREEN_LEFT, "task2: %d", i);
    }

    while(true){
        io_hlt();
    }
}

void kernel_entry(uint32_t magic, MULTIBOOT_INFO *multiboot_info)
{

    io_cli();
    init_gdtidt();
    init_pit();
    init_pic();
    init_memory();
    init_inthandler();
    io_sti();

    printf(TEXT_MODE_SCREEN_LEFT, "hello");
/*     integer_puts(multiboot_info->mem_upper, 21); */
/*     integer_puts(multiboot_info->mmap_addr, 22); */
/*     integer_puts(multiboot_info->mmap_length, 23); */
/*     list_test(); */

    set_task(0, NULL, NULL);
    set_task(1, task1, stack[0]+1024);
    set_task(2, task2, stack[1]+1024);

    for(;;){
                io_hlt();
/*         *         io_cli(); +| */
        if (keyboard_data_queue_check()) {
/*             io_sti(); */
        }
        else {
            /*             task_switch_c(0, 1); */
            /*             printf(TEXT_MODE_SCREEN_RIGHT, "test"); */
            /*             print_array_status(); */
        }
    }

}


