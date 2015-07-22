#include "process.h"
#include "kmemory.h"
#include "system.h"
#include "trap.h"
#include "segment.h"

#include "kernel.h"
#include <string.h>

/* static task_struct task_struct_head; */
static task_struct* current_task;

void test_thread(void)
{
    printk("start test thread");

    while (true) {
        io_hlt();
    }
}

extern uint32_t test_stack_start;

bool init_process(void)
{
    /*     task_struct_head = (task_struct* )kmalloc(sizeof(task_struct)); */
    current_task = (task_struct*)kmalloc(sizeof(task_struct));
    current_task->next_ptr = current_task;

    /*     current_task->= (task_struct*)kmalloc(sizeof(task_struct)); */

    current_task->page_directory_table = (uintptr_t)NULL;
    current_task->context.eip = (uintptr_t)NULL;
    current_task->context.esp = (uintptr_t)&system_stack_start;

    *(uint32_t*)(current_task->context.esp - 0) =
        0x0000202;  // eflags //TODO: this flags is not collect.
    *(uint32_t*)(current_task->context.esp - 0x4) =
        KERNEL_CODE_SEGMENT_INDEX * 0x8;                     // cs
    *(uint32_t*)(current_task->context.esp - 0x8) = system;  // eip

    *(uint32_t*)(current_task->context.esp - 0xc) = 0x77777777;
    *(uint32_t*)(current_task->context.esp - 0x10) = 0x66666666;
    *(uint32_t*)(current_task->context.esp - 0x14) = 0x55555555;
    *(uint32_t*)(current_task->context.esp - 0x18) = 0x44444444;
    *(uint32_t*)(current_task->context.esp - 0x1c) = 0x33333333;
    *(uint32_t*)(current_task->context.esp - 0x20) = 0x22222222;
    *(uint32_t*)(current_task->context.esp - 0x24) = 0x11111111;
    *(uint32_t*)(current_task->context.esp - 0x28) = 0x00000000;

    strcpy(current_task->name, "system");

    printk("system_stack :0x%x", &system_stack_start);

    // below code is sample
    task_struct* test_thread_struct =
        (task_struct*)kmalloc(sizeof(task_struct));
    test_thread_struct->page_directory_table = (uintptr_t)NULL;
    test_thread_struct->context.eip = (uintptr_t)restart;
    test_thread_struct->context.esp = (uintptr_t)&test_stack_start;

    *(uint32_t*)(test_thread_struct->context.esp - 0) = 0x0000202;
    // TODO: this flags is not collect. //eflags
    *(uint32_t*)(test_thread_struct->context.esp - 0x4) =
        KERNEL_CODE_SEGMENT_INDEX * 0x8;  // cs
    *(uint32_t*)(test_thread_struct->context.esp - 0x8) = test_thread;
    // eip */

    *(uint32_t*)(test_thread_struct->context.esp - 0xc) = 0x77777777;
    *(uint32_t*)(test_thread_struct->context.esp - 0x10) = 0x66666666;
    *(uint32_t*)(test_thread_struct->context.esp - 0x14) = 0x55555555;
    *(uint32_t*)(test_thread_struct->context.esp - 0x18) = 0x44444444;
    *(uint32_t*)(test_thread_struct->context.esp - 0x1c) = 0x33333333;
    *(uint32_t*)(test_thread_struct->context.esp - 0x20) = 0x22222222;
    *(uint32_t*)(test_thread_struct->context.esp - 0x24) = 0x11111111;
    *(uint32_t*)(test_thread_struct->context.esp - 0x28) = 0x00000000;
    strcpy(test_thread_struct->name, "test");

    printk("system_stack :0x%x", test_thread_struct->context.esp);
    test_thread_struct->context.esp -= 0x28;
    printk("system_stack :0x%x", test_thread_struct->context.esp);


    append_take_node(current_task, test_thread_struct);


    return true;
}

bool start(void)
{
    start_tasks(current_task->context.esp - 0x28);

    // never reached
    return false;
}

void task_switch(task_struct* prev, task_struct* next)
{
    asm volatile(
        ".text;"
        ".align 16;"
        "pushfl;"
        "pushl %%ebp;"
        "movl %%esp, %[prev_esp];"
        "movl %[next_esp], %%esp;"
        "movl $1f, %[prev_eip];"
        "pushl %[next_eip];"
        "jmp __switch_to;"
        "1:;"
        "popl %%ebp;"
        "popfl;"
        // output parameters
        : [prev_esp] "=m"(prev->context.esp), [prev_eip] "=m"(prev->context.eip)
        // input parameters
        : [next_esp] "m"(next->context.esp), [next_eip] "m"(next->context.eip),
          "a"(prev), "d"(next));
}

void __switch_to(void)
{
    /*     io_sti(); */
}

void schedule(void)
{
    task_struct* prev = current_task;
    task_struct* next = current_task->next_ptr;

    /*     printk("0x%x", current_task->context.esp); */
    printk("switch: %s -> %s", prev->name, next->name);

    current_task = current_task->next_ptr;
    /*     static int f = 0;  */
    /*     if(f == 0){ */
    task_switch(prev, next);
    /*         f = 1; */
    /*     } */
    /*     else{ */
    /*         task_switch(current_task->next_ptr, current_task); */
    /*         f = 0; */
    /*     } */
    /*     if (prev != next) { */
    /*     } */
}

void append_take_node(task_struct* head, task_struct* node)
{
    task_struct* tmp = head;
    while (tmp->next_ptr != head) {
        tmp = tmp->next_ptr;
    }

    node->next_ptr = head;
    tmp->next_ptr = node;
}
