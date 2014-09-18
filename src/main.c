#include"graphic.h"
#include"segment.h"
#include"func.h"
#include"multiboot.h"
#include"k_memory.h"
#include"lib.h"
#include"interrupt_handler.h"
#include"task.h"


/* uint8_t stack[3][1024]; */
uint32_t stack[1024];

void task1(void)
{
    int i = 0;
    io_sti();
/*     printf(TEXT_MODE_SCREEN_RIGHT, "0x%x", io_load_eflags()); */
    for (i = 0; i < 100000; i++) {
        printf(TEXT_MODE_SCREEN_RIGHT, "task1: %d", i);
    }

    while(true){
        io_hlt();
    }
}

void task2(void)
{
    int i = 0;
    io_sti();
    for (i = 0; i < 100000; i++) {
        printf(TEXT_MODE_SCREEN_RIGHT, "task2: %d", i);
    }

    while(true){
        io_hlt();
    }
}

void kernel_entry(uint32_t magic, MULTIBOOT_INFO *multiboot_info)
{

    io_cli();
    init_screen();

    init_gdtidt();

     if(!init_memory(multiboot_info)){
        //TODO: panic
        printf(TEXT_MODE_SCREEN_RIGHT, "-----------------kernel panic-----------------");
        io_hlt();
    }

    init_pit();
    init_pic();
    init_inthandler();

    enable_paging();
    io_sti();


/*     printf(TEXT_MODE_SCREEN_LEFT, "hello"); */
/*     printf(TEXT_MODE_SCREEN_LEFT, "mem_lower: %d(KB)", multiboot_info->mem_lower); */
/*     printf(TEXT_MODE_SCREEN_LEFT, "mem_upper: %d(KB)", multiboot_info->mem_upper); */
/*     printf(TEXT_MODE_SCREEN_LEFT, "mem_total: %d(KB)", (multiboot_info->mem_upper + multiboot_info->mem_lower + 1024)); */

/*     set_task(0, NULL, NULL); */
/*     set_task(1, task1, stack[0]+1024); */
/*     set_task(2, task2, stack[1]+1024); */

/*     printf(TEXT_MODE_SCREEN_LEFT, "%x", &_kernel_end); */
/*     printf(TEXT_MODE_SCREEN_LEFT, "%x", &_kernel_start); */

    for(;;){
        io_hlt();
        /*    io_cli();  */
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




