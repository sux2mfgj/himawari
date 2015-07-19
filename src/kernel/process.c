#include "process.h"
#include "kmemory.h"
#include "system.h"

#include "kernel.h"

/* static task_struct task_struct_head; */
static task_struct* current_task;

void test_thread(void)
{
    printk("start test thread");

    while (true) {
        printk("test thread main loop");
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
    current_task->context.eip = (uintptr_t)system;
    current_task->context.esp = (uintptr_t)NULL;
    current_task->name = "";
    
    /*     task_struct_head.next_ptr = &task_struct_head; */

    /*     task_struct_head= (task_struct* )kmalloc(sizeof(task_struct)); */

/*     task_struct_head.page_directory_table = (uintptr_t)NULL; */
/*     task_struct_head.context.eip = (uintptr_t)system; */
/*     task_struct_head.context.esp = (uintptr_t)NULL; */

/*     current_node = &task_struct_head; */

    // below code is sample
    task_struct* test_thread_struct = (task_struct *)kmalloc(sizeof(task_struct));

/*     test_thread->task_struct = (task_struct*)kmalloc(sizeof(task_struct)); */

    test_thread_struct->page_directory_table = (uintptr_t)NULL;
    test_thread_struct->context.eip = (uintptr_t)test_thread;
    test_thread_struct->context.esp = (uintptr_t)test_stack_start;

/*     task_node* test_thread_node = kmalloc(sizeof(task_node)); */

/*     test_thread_node->task_struct = (task_struct*)kmalloc(sizeof(task_struct)); */

/*     test_thread_node->task_struct->page_directory_table = NULL; */
/*     test_thread_node->task_struct->context.eip = test_thread; */
/*     test_thread_node->task_struct->context.esp = test_stack_start; */

    append_take_node(current_task, test_thread_struct);

    return true;
}

void task_switch(task_struct *prev, task_struct *next)
{
    asm volatile(
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
            //output parameters
            :[prev_esp] "=m" (prev->context.esp), [prev_eip] "=m" (prev->context.eip)
            //input parameters
            :[next_esp] "m" (next->context.esp), [next_eip] "m" (next->context.eip),
            "a" (prev), "d" (next));
}

void __switch_to(void) {}

void schedule(void)
{
    task_struct* prev = current_task;
    task_struct* next = current_task->next_ptr;

    if (prev != next){
/*         printk("0x%x", current_node->next_ptr->task_struct->context.esp); */
/*         printk("0x%x", current_node->next_ptr->task_struct->context.eip); */
        task_switch(prev, next);
    }
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
