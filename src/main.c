#include "graphic.h"
#include "segment.h"
#include "func.h"
#include "multiboot.h"
#include "k_memory.h"
#include "lib.h"
#include "interrupt_handler.h"
#include "task.h"
#include "trap.h"

void task1(void)
{
    int i = 100000;
    while (i>0) {
        printk(DEBUG1, "task1: %d", i++);
    }
    while (true) {
        io_hlt();
    }

}

void task2(void)
{
    int i = 0;
    while (i >= 0) {
        printk(DEBUG2, "task2: %d", i++);
    }

    while (true) {
        io_hlt();
    }
}

void kernel_entry(uint32_t magic, MULTIBOOT_INFO *multiboot_info)
{
    io_cli();
    init_screen();

    init_gdtidt();
    init_interrupt();

    if (!init_memory(multiboot_info)) {
        // TODO: panic
        printf(TEXT_MODE_SCREEN_RIGHT, "------------kernel panic------------");
        io_hlt();
    }

    init_pit();
    init_pic();
    init_inthandler();

    //init_tss();

    init_task();
    create_kernel_thread(init);
/*     print_pid_test(); */

    io_sti();
/*     printf(TEXT_MODE_SCREEN_RIGHT, "----start----"); */

    /*     printf(TEXT_MODE_SCREEN_RIGHT, "hello"); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "mem_lower: %d(KB)",
     * multiboot_info->mem_lower); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "mem_upper: %d(KB)",
     * multiboot_info->mem_upper); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "mem_total: %d(KB)",
     * (multiboot_info->mem_upper + multiboot_info->mem_lower + 1024)); */

    /*     printf(TEXT_MODE_SCREEN_RIGHT, "%x", &_kernel_end); */
    /*     printf(TEXT_MODE_SCREEN_RIGHT, "%x", &_kernel_start); */

    /*     uint32_t stack[2][1024]; */
    /*     task_struct kernel_task, task_struct0, task_struct1; */
    /*     task_struct0.context.eip = (uintptr_t)task1; */
    /*     task_struct0.context.esp = (uintptr_t)stack[0] +
     * sizeof(uint32_t)*1024; */
    /*     task_struct1.context.eip = (uintptr_t)task2; */
    /*     task_struct1.context.esp = (uintptr_t)stack[1] +
     * sizeof(uint32_t)*1024; */
    /*     task_switch((task_struct*)&kernel_task, (task_struct*)&task_struct0);
     */

    for (;;) {
        io_hlt();
/*         if (keyboard_data_queue_check()) { */
/*             io_sti(); */
/*         } else { */
/*         } */
    }
}

